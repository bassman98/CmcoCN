import pathlib, re
root=pathlib.Path('.').resolve()
patterns = ['*.h','*.hpp','*.ino','*.cpp']
for p in sorted(root.rglob('*')):
    if not any(p.match('**/'+pat) for pat in patterns):
        continue
    text = p.read_text(encoding='utf-8', errors='ignore')
    ifs = len(re.findall(r'(?m)^\s*#\s*(if|ifdef|ifndef)\b', text))
    endifs = len(re.findall(r'(?m)^\s*#\s*endif\b', text))
    if ifs != endifs:
        print(f"{ifs} != {endifs} : {p}")
