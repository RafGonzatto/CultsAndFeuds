import pathlib

def main():
    root = pathlib.Path(r'c:/Users/Windows 11/Desktop/UNREAL/Vazio/Source/Vazio')
    category_map = {
        'LogEnemySpawn': 'LOG_ENEMIES',
        'LogEnemy': 'LOG_ENEMIES',
        'LogBoss': 'LOG_ENEMIES',
        'LogBossAutoTest': 'LOG_ENEMIES',
        'LogEconomy': 'LOG_UPGRADES',
        'LogXP': 'LOG_UPGRADES',
        'LogXPOrb': 'LOG_UPGRADES',
        'LogPlayerUI': 'LOG_UI',
        'LogPlayerHealth': 'LOG_WEAPONS',
    }
    level_map = {
        'Fatal': 'Error',
        'Error': 'Error',
        'Warning': 'Warn',
        'Display': 'Info',
        'Log': 'Info',
        'Verbose': 'Debug',
        'VeryVerbose': 'Trace',
        'All': 'Debug',
    }
    converted_files = []
    for path in root.rglob('*.[ch]pp'):
        text = path.read_text(encoding='utf-8')
        changed = False
        result = []
        i = 0
        while i < len(text):
            idx = text.find('UE_LOG(', i)
            if idx == -1:
                result.append(text[i:])
                break
            result.append(text[i:idx])
            j = idx + len('UE_LOG(')
            depth = 1
            while j < len(text) and depth > 0:
                ch = text[j]
                if ch == '(':
                    depth += 1
                elif ch == ')':
                    depth -= 1
                j += 1
            if depth != 0:
                result.append(text[idx:])
                i = len(text)
                break
            call_inside = text[idx + len('UE_LOG('): j - 1]
            semi_idx = j
            while semi_idx < len(text) and text[semi_idx] in '\n\r\t ':
                semi_idx += 1
            if semi_idx < len(text) and text[semi_idx] == ';':
                end_idx = semi_idx + 1
            else:
                end_idx = semi_idx
            args = []
            curr = []
            depth_arg = 0
            for ch in call_inside:
                if ch == ',' and depth_arg == 0:
                    args.append(''.join(curr).strip(' \t\n\r'))
                    curr = []
                    continue
                if ch == '(':
                    depth_arg += 1
                elif ch == ')':
                    depth_arg -= 1
                curr.append(ch)
            if curr:
                args.append(''.join(curr).strip(' \t\n\r'))
            if len(args) < 3:
                result.append(text[idx:end_idx])
                i = end_idx
                continue
            category, verbosity = args[0], args[1]
            if category not in category_map:
                result.append(text[idx:end_idx])
                i = end_idx
                continue
            macro = category_map[category]
            level = level_map.get(verbosity, 'Info')
            remaining = args[2:]
            args_joined = ', '.join(remaining)
            indent_idx = text.rfind('\n', 0, idx)
            indent = text[indent_idx+1:idx]
            new_call = f"{indent}{macro}({level}, {args_joined});"
            result.append(new_call)
            i = end_idx
            changed = True
        if changed:
            path.write_text(''.join(result), encoding='utf-8')
            converted_files.append(str(path.relative_to(root.parent)))
    if converted_files:
        print('Converted logs in:')
        for f in converted_files:
            print(f)

if __name__ == '__main__':
    main()
