$ErrorActionPreference = 'Stop'
$target = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '23120037-23120068-23120074-23120154.docx'))
$word = $null
$report = $null
$createdWord = $false
$openedReport = $false
try {
    try { $word = [Runtime.InteropServices.Marshal]::GetActiveObject('Word.Application') }
    catch {
        $word = New-Object -ComObject Word.Application
        $word.Visible = $false
        $createdWord = $true
    }
    foreach ($document in $word.Documents) {
        if ($document.FullName -eq $target) { $report = $document; break }
    }
    if ($null -eq $report) {
        $report = $word.Documents.Open($target, $false, $false)
        $openedReport = $true
    }
    if ($report.ReadOnly) { throw 'Report is read-only; no changes applied.' }
    if ($report.Content.Text -notmatch '(?m)^\[7\]\s') {
        throw 'Bibliography is already revised or has changed; do not apply the old numbering again.'
    }

    # Remove only internal bibliography entries, working backwards.
    for ($i = $report.Paragraphs.Count; $i -ge 1; --$i) {
        $p = $report.Paragraphs.Item($i)
        if ($p.Range.Text.Trim() -match '^\[(1|2|4)\]\s') { $p.Range.Delete() | Out-Null }
    }
    function Replace-Text([string]$before, [string]$after) {
        $range = $report.Content
        $range.Find.ClearFormatting()
        $range.Find.Replacement.ClearFormatting()
        [void]$range.Find.Execute($before, $false, $false, $false, $false, $false, $true, 1, $false, $after, 2)
    }
    # Placeholders avoid renumbering a citation twice.
    foreach ($pair in @(@(3,1), @(5,2), @(6,3), @(7,4))) {
        # Require the leading space in running text so digits[7] stays unchanged.
        Replace-Text " [$($pair[0])]" " [CITATION_$($pair[1])]"
        foreach ($p in $report.Paragraphs) {
            if ($p.Range.Text -match "^\[$($pair[0])\]\s") {
                $r = $p.Range.Duplicate
                [void]$r.Find.Execute("[$($pair[0])]", $false, $false, $false, $false, $false, $true, 0, $false, "[CITATION_$($pair[1])]", 1)
            }
        }
    }
    foreach ($old in @(1,2,4)) { Replace-Text " [$old]" '' }
    foreach ($new in 1..4) { Replace-Text "[CITATION_$new]" "[$new]" }

    # Read the confirmed task assignments from the synchronized UTF-8 source.
    $assignments = @{}
    foreach ($line in (Get-Content (Join-Path $PSScriptRoot 'report_content.md') -Encoding UTF8)) {
        if ($line -match '^\| `231\d+` \|') {
            $cells = $line.Trim([char]'|').Split([char]'|')
            $id = $cells[0].Trim().Trim([char]96)
            $assignments[$id] = $cells[2].Trim().Replace([string][char]96, '')
        }
    }
    if ($assignments.Count -ne 4) { throw 'Expected exactly four confirmed members.' }
    $updated = 0
    foreach ($table in $report.Tables) {
        if ($table.Columns.Count -ne 4) { continue }
        for ($row = 2; $row -le $table.Rows.Count; ++$row) {
            $id = $table.Cell($row,1).Range.Text.Trim([char[]]@(13,7,32))
            if (!$assignments.ContainsKey($id)) { continue }
            foreach ($col in @(3,4)) {
                $range = $table.Cell($row,$col).Range.Duplicate
                $range.End = $range.End - 1
                $range.Text = $(if ($col -eq 3) { $assignments[$id] } else { '100%' })
                $range.Font.Name = 'Times New Roman'
                $range.Font.Size = 13
                $range.Font.Italic = 0
                $range.Font.Color = 0
            }
            foreach ($token in @('modular_exponentiation','generate_safe_prime','generate_private_key')) {
                $range = $table.Cell($row,3).Range.Duplicate
                if ($range.Find.Execute($token)) { $range.Font.Name = 'Courier New'; $range.Font.Size = 13 }
            }
            ++$updated
        }
    }
    if ($updated -ne 4) { throw "Updated $updated member rows; expected four." }
    for ($i = $report.Paragraphs.Count; $i -ge 1; --$i) {
        $p = $report.Paragraphs.Item($i)
        if ($p.Range.Text.StartsWith('Ghi ') -and $p.Range.Font.Italic -eq -1) {
            $p.Range.Delete() | Out-Null
        }
    }
    $report.Repaginate()
    foreach ($toc in $report.TablesOfContents) { $toc.Update() }
    $report.Fields.Update() | Out-Null
    $report.Repaginate()
    foreach ($toc in $report.TablesOfContents) { $toc.UpdatePageNumbers() }
    $report.Save()
    Write-Output "Updated citations and $updated member rows; $($report.ComputeStatistics(2)) pages."
}
finally {
    if ($null -ne $report -and $openedReport) { $report.Close(0) }
    if ($null -ne $word -and $createdWord) { $word.Quit() }
}
