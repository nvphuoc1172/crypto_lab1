"""Build the group DOCX from the edited template and reviewed report content."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from xml.dom import minidom as D
import re

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'docs/reports/MAU_BAO_CAO_LAB.docx'
OUTPUT = ROOT / 'docs/reports/23120037-23120068-23120074-23120154.docx'
CONTENT = Path(__file__).with_name('report_content.md')
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'


def child(parent, tag):
    found = next((n for n in parent.childNodes
                  if n.nodeType == n.ELEMENT_NODE and n.tagName == tag), None)
    if found is None:
        found = parent.ownerDocument.createElementNS(W, tag)
        parent.appendChild(found)
    return found


def value(parent, tag, val):
    child(parent, tag).setAttribute('w:val', str(val))


def font(rp, name='Times New Roman', size=26):
    f = child(rp, 'w:rFonts')
    for a in ['asciiTheme', 'hAnsiTheme', 'eastAsiaTheme', 'cstheme']:
        if f.hasAttribute('w:' + a):
            f.removeAttribute('w:' + a)
    for a in ['ascii', 'hAnsi', 'eastAsia', 'cs']:
        f.setAttribute('w:' + a, name)
    value(rp, 'w:sz', size)
    value(rp, 'w:szCs', size)


with ZipFile(SOURCE) as zin:
    entries = [(i, zin.read(i.filename)) for i in zin.infolist()]
raw = {i.filename: data for i, data in entries}
doc = D.parseString(raw['word/document.xml'])
styles = D.parseString(raw['word/styles.xml'])
body = doc.getElementsByTagName('w:body')[0]
original = [n for n in body.childNodes if n.nodeType == n.ELEMENT_NODE]
cover = [n.cloneNode(True) for n in original[:27]]
section = original[-1].cloneNode(True)

# Change only the lab-title text, retaining that paragraph's original appearance.
title = cover[9]
r = next((n for n in title.childNodes if n.nodeType == n.ELEMENT_NODE and n.tagName == 'w:r'), None)
rp = child(r, 'w:rPr').cloneNode(True) if r else doc.createElementNS(W, 'w:rPr')
for n in list(title.childNodes):
    if n.nodeType != n.ELEMENT_NODE or n.tagName != 'w:pPr':
        title.removeChild(n)
r = child(title, 'w:r')
r.appendChild(rp)
child(r, 'w:t').appendChild(doc.createTextNode('LAB01: SỐ NGUYÊN TỐ VÀ TRAO ĐỔI KHÓA DIFFIE–HELLMAN'))

# Body defaults already match the template; make the requested styles explicit.
for st in styles.getElementsByTagName('w:style'):
    sid = st.getAttribute('w:styleId')
    if sid in ['Binhthng', 'Code', 'CodeInline', 'u1', 'u2', 'u3']:
        rp = child(st, 'w:rPr')
        size = {'u1': 32, 'u2': 28, 'u3': 26}.get(sid, 26)
        font(rp, 'Courier New' if sid in ['Code', 'CodeInline'] else 'Times New Roman', size)
        if sid in ['u1', 'u2', 'u3']:
            value(rp, 'w:b', int(sid != 'u3'))
            value(rp, 'w:bCs', int(sid != 'u3'))
            value(rp, 'w:caps', int(sid == 'u1'))
            value(rp, 'w:i', 0)
            value(rp, 'w:iCs', 0)

for n in list(body.childNodes):
    body.removeChild(n)
for p in cover:
    body.appendChild(p)


def make_run(parent, text, code=False, bold=False, note=False, size=None):
    r = doc.createElementNS(W, 'w:r')
    rp = child(r, 'w:rPr')
    if code:
        value(rp, 'w:rStyle', 'CodeInline')
    font(rp, 'Courier New' if code else 'Times New Roman', size or (24 if note else 26))
    if bold:
        value(rp, 'w:b', 1)
    if note:
        value(rp, 'w:i', 1)
        value(rp, 'w:iCs', 1)
        value(rp, 'w:color', '92D050')
    t = child(r, 'w:t')
    t.setAttribute('xml:space', 'preserve')
    t.appendChild(doc.createTextNode(text))
    parent.appendChild(r)


def rich(parent, text, note=False, bold=False):
    for part in re.split(r'(`[^`]+`)', text):
        if part:
            code = part.startswith('`') and part.endswith('`')
            make_run(parent, part[1:-1] if code else part, code, bold, note)


def paragraph(text='', style='Binhthng', parent=body, code=False, note=False):
    p = doc.createElementNS(W, 'w:p')
    pp = child(p, 'w:pPr')
    value(pp, 'w:pStyle', style)
    value(pp, 'w:widowControl', 1)
    if style == 'Binhthng':
        value(pp, 'w:jc', 'both')
        # Keep the template's 1.15 line spacing, with modest paragraph separation.
        sp = child(pp, 'w:spacing')
        sp.setAttribute('w:line', '276')
        sp.setAttribute('w:lineRule', 'auto')
        sp.setAttribute('w:after', '120')
    if code:
        value(pp, 'w:jc', 'left')
        sp = child(pp, 'w:spacing')
        sp.setAttribute('w:line', '240')
        sp.setAttribute('w:lineRule', 'auto')
        sp.setAttribute('w:after', '0')
        make_run(p, text, code=True)
    else:
        rich(p, text, note=note)
    parent.appendChild(p)
    return p


def page_break():
    p = paragraph()
    child(child(p, 'w:r'), 'w:br').setAttribute('w:type', 'page')


# TOC field with valid cached entries; Word updates page numbers on opening.
toc_title = original[27].cloneNode(True)
for tag in ['w:bookmarkStart', 'w:bookmarkEnd', 'w:lastRenderedPageBreak']:
    for n in list(toc_title.getElementsByTagName(tag)):
        n.parentNode.removeChild(n)
body.appendChild(toc_title)
p = paragraph()
r = child(p, 'w:r')
child(r, 'w:fldChar').setAttribute('w:fldCharType', 'begin')
r = doc.createElementNS(W, 'w:r')
t = child(r, 'w:instrText')
t.setAttribute('xml:space', 'preserve')
t.appendChild(doc.createTextNode(' TOC \\o "1-2" \\h \\z \\u '))
p.appendChild(r)
r = doc.createElementNS(W, 'w:r')
child(r, 'w:fldChar').setAttribute('w:fldCharType', 'separate')
p.appendChild(r)
toc_position = len(body.childNodes)

toc_entries = []
head_count = 0
sub_count = 0


def heading(text, level):
    global head_count, sub_count
    if level == 1:
        head_count += 1
        sub_count = 0
        number = str(head_count)
    else:
        sub_count += 1
        number = f'{head_count}.{sub_count}'
    p = paragraph(style='u' + str(level))
    pp = child(p, 'w:pPr')
    # Literal section numbers avoid inherited numbering collisions in the template.
    value(pp, 'w:keepNext', 1)
    if level == 1 and head_count > 1:
        value(pp, 'w:pageBreakBefore', 1)
    anchor = f'ReportSection{len(toc_entries)+1}'
    mark = doc.createElementNS(W, 'w:bookmarkStart')
    mark.setAttribute('w:id', str(100 + len(toc_entries)))
    mark.setAttribute('w:name', anchor)
    p.appendChild(mark)
    # Font/size comes from heading style, not a body run override.
    r = doc.createElementNS(W, 'w:r')
    child(r, 'w:t').appendChild(doc.createTextNode(number + '. ' + (text.upper() if level == 1 else text)))
    p.appendChild(r)
    end = doc.createElementNS(W, 'w:bookmarkEnd')
    end.setAttribute('w:id', mark.getAttribute('w:id'))
    p.appendChild(end)
    toc_entries.append((number + '. ' + text, anchor, level))


def table(rows):
    cols = len(rows[0])
    widths = {2: [3700, 5329], 3: [2200, 4400, 2429], 4: [1500, 2500, 3200, 1829]}[cols]
    tbl = doc.createElementNS(W, 'w:tbl')
    pr = child(tbl, 'w:tblPr')
    tw = child(pr, 'w:tblW')
    tw.setAttribute('w:w', str(sum(widths)))
    tw.setAttribute('w:type', 'dxa')
    value(pr, 'w:tblLayout', 'fixed')
    borders = child(pr, 'w:tblBorders')
    for edge in ['top', 'left', 'bottom', 'right', 'insideH', 'insideV']:
        n = child(borders, 'w:' + edge)
        n.setAttribute('w:val', 'single')
        n.setAttribute('w:sz', '4')
        n.setAttribute('w:color', 'B7B7B7')
    margins = child(pr, 'w:tblCellMar')
    for side in ['top', 'left', 'bottom', 'right']:
        n = child(margins, 'w:' + side)
        n.setAttribute('w:w', '80')
        n.setAttribute('w:type', 'dxa')
    grid = child(tbl, 'w:tblGrid')
    for width in widths:
        child_node = doc.createElementNS(W, 'w:gridCol')
        child_node.setAttribute('w:w', str(width))
        grid.appendChild(child_node)
    for index, row in enumerate(rows):
        tr = doc.createElementNS(W, 'w:tr')
        trp = child(tr, 'w:trPr')
        value(trp, 'w:cantSplit', 1)
        if index == 0:
            value(trp, 'w:tblHeader', 1)
        for width, text in zip(widths, row):
            tc = doc.createElementNS(W, 'w:tc')
            tcp = child(tc, 'w:tcPr')
            cw = child(tcp, 'w:tcW')
            cw.setAttribute('w:w', str(width))
            cw.setAttribute('w:type', 'dxa')
            if index == 0:
                child(tcp, 'w:shd').setAttribute('w:fill', 'E7E6E6')
            p = paragraph(parent=tc)
            value(child(p, 'w:pPr'), 'w:jc', 'left')
            rich(p, text, bold=index == 0)
            tr.appendChild(tc)
        tbl.appendChild(tr)
    body.appendChild(tbl)
    paragraph()


# Finish TOC field now; cached entries will be inserted before its end.
toc_end = paragraph()
child(child(toc_end, 'w:r'), 'w:fldChar').setAttribute('w:fldCharType', 'end')
page_break()

lines = CONTENT.read_text(encoding='utf-8').splitlines()
i = 0
while i < len(lines):
    line = lines[i].strip()
    if not line:
        i += 1
        continue
    if line.startswith('```'):
        i += 1
        while i < len(lines) and not lines[i].startswith('```'):
            paragraph(lines[i], 'Code', code=True)
            i += 1
        paragraph()
    elif line.startswith('# '):
        heading(line[2:], 1)
    elif line.startswith('## '):
        heading(line[3:], 2)
    elif line.startswith('|'):
        rows = []
        while i < len(lines) and lines[i].strip().startswith('|'):
            row = [c.strip() for c in lines[i].strip().strip('|').split('|')]
            if not all(re.fullmatch(r'[-: ]+', c) for c in row):
                rows.append(row)
            i += 1
        table(rows)
        continue
    elif line.startswith('!'):
        paragraph(line[1:], note=True)
    else:
        paragraph(line)
    i += 1

for text, anchor, level in toc_entries:
    p = doc.createElementNS(W, 'w:p')
    pp = child(p, 'w:pPr')
    value(pp, 'w:pStyle', 'Binhthng')
    if level > 1:
        child(pp, 'w:ind').setAttribute('w:left', '360')
    sp = child(pp, 'w:spacing')
    sp.setAttribute('w:after', '60')
    tabs = child(pp, 'w:tabs')
    tab = child(tabs, 'w:tab')
    tab.setAttribute('w:val', 'right')
    tab.setAttribute('w:leader', 'dot')
    tab.setAttribute('w:pos', '8800')
    link = child(p, 'w:hyperlink')
    link.setAttribute('w:anchor', anchor)
    make_run(link, text)
    child(child(p, 'w:r'), 'w:tab')
    field = child(p, 'w:fldSimple')
    field.setAttribute('w:instr', f' PAGEREF {anchor} \\h ')
    make_run(field, '—')
    body.insertBefore(p, toc_end)

body.appendChild(section)
settings = D.parseString(raw['word/settings.xml'])
value(settings.documentElement, 'w:updateFields', 'true')
changes = {
    'word/document.xml': doc.toxml(encoding='UTF-8'),
    'word/styles.xml': styles.toxml(encoding='UTF-8'),
    'word/settings.xml': settings.toxml(encoding='UTF-8'),
}
# Avoid stale author/comment metadata in the new group document.
if 'docProps/core.xml' in raw:
    core = D.parseString(raw['docProps/core.xml'])
    for tag in ['dc:creator', 'cp:lastModifiedBy']:
        for n in core.getElementsByTagName(tag):
            for c in list(n.childNodes):
                n.removeChild(c)
            n.appendChild(core.createTextNode('Nhóm 23120037-23120068-23120074-23120154'))
    changes['docProps/core.xml'] = core.toxml(encoding='UTF-8')
with ZipFile(OUTPUT, 'w', ZIP_DEFLATED) as zout:
    for info, data in entries:
        zout.writestr(info, changes.get(info.filename, data))
with ZipFile(OUTPUT) as z:
    assert z.testzip() is None
    for name in z.namelist():
        if name.endswith('.xml'):
            D.parseString(z.read(name))
assert head_count == 10
print(f'Created {OUTPUT} with {head_count} main sections and {len(toc_entries)} TOC entries.')
