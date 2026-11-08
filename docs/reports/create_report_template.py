"""Create a blank report template while preserving the supplied Word layout."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from xml.dom import minidom as D

import argparse

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source', type=Path, required=True)
parser.add_argument('--output', type=Path, default=ROOT / 'docs/reports/MAU_BAO_CAO_LAB_GENERATED.docx')
args = parser.parse_args()
SOURCE, OUTPUT = args.source, args.output
if not SOURCE.is_file():
    parser.error('Source DOCX does not exist')
if OUTPUT.exists():
    parser.error('Output already exists; choose a new path to preserve manual edits')
OUTPUT.parent.mkdir(parents=True, exist_ok=True)
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

def elements(node, tag):
    return list(node.getElementsByTagName(tag))

def direct(node, tag):
    return next((n for n in node.childNodes if n.nodeType == n.ELEMENT_NODE and n.tagName == tag), None)

def child(node, tag):
    existing = direct(node, tag)
    if existing is None:
        existing = node.ownerDocument.createElementNS(W, tag)
        node.appendChild(existing)
    return existing

def typography(rpr, font='Times New Roman', size=26):
    f = child(rpr, 'w:rFonts')
    for a in ['ascii', 'hAnsi', 'eastAsia', 'cs']:
        f.setAttribute('w:' + a, font)
    for a in ['asciiTheme', 'hAnsiTheme', 'eastAsiaTheme', 'cstheme']:
        f.removeAttribute('w:' + a) if f.hasAttribute('w:' + a) else None
    for tag in ['w:sz', 'w:szCs']:
        child(rpr, tag).setAttribute('w:val', str(size))

def replace_text(p, text, body=False):
    first_run = direct(p, 'w:r')
    rp = direct(first_run, 'w:rPr') if first_run is not None else None
    rp = rp.cloneNode(True) if rp is not None else p.ownerDocument.createElementNS(W, 'w:rPr')
    for n in list(p.childNodes):
        if n.nodeType != n.ELEMENT_NODE or n.tagName != 'w:pPr':
            p.removeChild(n)
    if body:
        typography(rp)
        typography(child(child(p, 'w:pPr'), 'w:rPr'))
    r = child(p, 'w:r')
    r.appendChild(rp)
    t = child(r, 'w:t')
    t.appendChild(p.ownerDocument.createTextNode(text))
    return p

with ZipFile(SOURCE) as z:
    doc = D.parseString(z.read('word/document.xml'))
    body = elements(doc, 'w:body')[0]
    original = [n for n in body.childNodes if n.nodeType == n.ELEMENT_NODE]
    section = original[-1].cloneNode(True)
    cover = [n.cloneNode(True) for n in original[:28]]
    replacements = {
        6: 'MÔN HỌC: Nhập môn Mã hóa - Mật mã',
        9: '[Tên bài Lab]', 10: '', 11: '',
        15: 'THÀNH VIÊN NHÓM:',
        16: 'Nguyễn Thanh Khánh Hà - 23120037',
        17: 'Lê Nguyễn Minh Nhật - 23120068',
        18: 'Nguyễn Văn Phước - 23120074',
        19: 'Nguyễn Thanh Phong - 23120154',
        20: 'LỚP: [Điền lớp học phần]',
        21: 'GIẢNG VIÊN HƯỚNG DẪN: [Điền họ tên]',
        27: 'THÀNH PHỐ HỒ CHÍ MINH - [Tháng/Năm]',
    }
    for i, text in replacements.items():
        text = text.upper()
        replace_text(cover[i], text)
        if i in [18, 19, 20, 21]:
            # Reuse the original member line's formatting for former blank lines.
            formatted = original[15].cloneNode(True)
            replace_text(formatted, text)
            cover[i] = formatted
    # Keep every cover label uppercase, including retained school headings.
    for p in cover:
        for t in elements(p, 'w:t'):
            for node in t.childNodes:
                if node.nodeType == node.TEXT_NODE:
                    node.data = node.data.upper()
    # Preserve the original explicit page break after the cover.
    r = child(cover[27], 'w:r')
    br = doc.createElementNS(W, 'w:br')
    br.setAttribute('w:type', 'page')
    r.appendChild(br)
    for n in list(body.childNodes):
        body.removeChild(n)
    for n in cover:
        body.appendChild(n)

    def para(text, heading=False):
        template = original[52 if heading else 58].cloneNode(True)
        replace_text(template, text, body=not heading)
        if not heading:
            ppr = child(template, 'w:pPr')
            ind = direct(ppr, 'w:ind')
            if ind is not None:
                ppr.removeChild(ind)
        body.appendChild(template)
        return template

    toc_title = original[28].cloneNode(True)
    body.appendChild(toc_title)
    toc = doc.createElementNS(W, 'w:p')
    field = child(toc, 'w:fldSimple')
    field.setAttribute('w:instr', 'TOC \\o "1-2" \\h \\z \\u')
    field.setAttribute('w:dirty', 'true')
    body.appendChild(toc)
    para('Cập nhật mục lục trong Word: nhấp phải mục lục, chọn Update Field / Update entire table.')
    page = doc.createElementNS(W, 'w:p')
    child(child(page, 'w:r'), 'w:br').setAttribute('w:type', 'page')
    body.appendChild(page)
    for title in [
        'Tổng quan bài Lab', 'Hệ thống xử lý số lớn', 'Lũy thừa mô-đun',
        'Kiểm tra nguyên tố và sinh số nguyên tố an toàn',
        'Trao đổi khóa Diffie–Hellman', 'Hướng dẫn biên dịch và chạy chương trình',
        'Kiểm thử và kết quả', 'Tự đánh giá và hướng cải thiện',
        'Tài liệu tham khảo',
    ]:
        para(title, heading=True)
        para('')
    para('Quy định trình bày báo cáo', heading=True)
    para('Nội dung báo cáo: Times New Roman, cỡ chữ 13 pt. Giữ căn lề, giãn dòng và bố cục theo mẫu; tiêu đề và trang bìa giữ định dạng riêng của mẫu.')
    para('Mã nguồn, tên hàm, tên biến và các giá trị liên quan đến code: Courier New, cỡ chữ 13 pt. Dùng style Code cho đoạn mã và CodeInline cho các thành phần nằm trong câu.')
    para('Các đề mục có thể sửa đổi theo bài Lab. Xóa hướng dẫn này khi hoàn thiện báo cáo và cập nhật lại mục lục trước khi xuất PDF.')
    body.appendChild(section)

    styles = D.parseString(z.read('word/styles.xml'))
    defaults = child(child(elements(styles, 'w:styles')[0], 'w:docDefaults'), 'w:rPrDefault')
    typography(child(defaults, 'w:rPr'))
    for s in elements(styles, 'w:style'):
        if s.getAttribute('w:styleId') in ['Normal', 'BodyText']:
            typography(child(s, 'w:rPr'))
    for sid, kind in [('Code', 'paragraph'), ('CodeInline', 'character')]:
        s = styles.createElementNS(W, 'w:style')
        s.setAttribute('w:type', kind)
        s.setAttribute('w:styleId', sid)
        s.setAttribute('w:customStyle', '1')
        child(s, 'w:name').setAttribute('w:val', sid)
        child(s, 'w:basedOn').setAttribute('w:val', 'Normal' if kind == 'paragraph' else 'DefaultParagraphFont')
        child(s, 'w:qFormat')
        typography(child(s, 'w:rPr'), 'Courier New')
        styles.documentElement.appendChild(s)

    header = z.read('word/header1.xml').decode('utf-8').replace('An ninh mạng - 23_4', 'Nhập môn Mã hóa - Mật mã')
    settings = D.parseString(z.read('word/settings.xml'))
    child(settings.documentElement, 'w:updateFields').setAttribute('w:val', 'true')
    changes = {
        'word/document.xml': doc.toxml(encoding='UTF-8'),
        'word/styles.xml': styles.toxml(encoding='UTF-8'),
        'word/header1.xml': header.encode('utf-8'),
        'word/settings.xml': settings.toxml(encoding='UTF-8'),
    }
    # Remove screenshots from the old report, retaining the cover logo only.
    rels = D.parseString(z.read('word/_rels/document.xml.rels'))
    removed = set()
    for rel in list(rels.documentElement.childNodes):
        if rel.nodeType == rel.ELEMENT_NODE and rel.getAttribute('Type').endswith('/image') and rel.getAttribute('Id') != 'rId7':
            removed.add('word/' + rel.getAttribute('Target'))
            rels.documentElement.removeChild(rel)
    changes['word/_rels/document.xml.rels'] = rels.toxml(encoding='UTF-8')
    with ZipFile(OUTPUT, 'w', ZIP_DEFLATED) as out:
        for item in z.infolist():
            if item.filename not in removed:
                out.writestr(item, changes.get(item.filename, z.read(item.filename)))

with ZipFile(OUTPUT) as out:
    assert out.testzip() is None
    for name in out.namelist():
        if name.endswith('.xml'):
            D.parseString(out.read(name))
    text = out.read('word/document.xml').decode('utf-8')
    assert all(s in text for s in ['23120037', '23120068', '23120074', '23120154'])
    assert 'LEO THANG' not in text and 'mimikatz' not in text
print(OUTPUT)
