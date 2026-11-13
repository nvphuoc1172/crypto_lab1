$ErrorActionPreference = 'Stop'
$reportPath = Join-Path $PSScriptRoot '23120037-23120068-23120074-23120154.docx'
$word = $null
$report = $null
try {
    $word = New-Object -ComObject Word.Application
    $word.Visible = $false
    $word.DisplayAlerts = 0
    $report = $word.Documents.Open($reportPath, $false, $false)
    $report.Repaginate()
    foreach ($toc in $report.TablesOfContents) { $toc.Update() }
    $report.Fields.Update() | Out-Null
    $report.Repaginate()
    foreach ($toc in $report.TablesOfContents) { $toc.UpdatePageNumbers() }
    $pages = $report.ComputeStatistics(2)
    $tocCount = $report.TablesOfContents.Count
    $report.Save()
    Write-Output "Word updated: $pages pages; $tocCount table(s) of contents."
}
finally {
    if ($null -ne $report) {
        $report.Close(0)
        [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($report)
    }
    if ($null -ne $word) {
        $word.Quit()
        [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($word)
    }
}
