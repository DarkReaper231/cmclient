import re
from pprint import pprint

RX_COMMAND = re.compile(r'(?P<returns>CommandCost|std::tuple<CommandCost, [^>]*>) (?P<name>Cmd\w*)\((?P<args>[^)]*)\);')
RX_ARG = re.compile(r'(?P<type>(:?const |)[\w:]* &?)(?P<name>\w*)')
RX_CAMEL_TO_SNAKE = re.compile(r'(?<!^)(?=[A-Z])')

FILES = [
    'src/order_cmd.h',
    'src/story_cmd.h',
]

OUTPUT = 'src/citymania/generated/cm_gen_commands'


def parse_commands():
    res = []
    for f in FILES:
        for returns, name, args_str in RX_COMMAND.findall(open(f).read()):
            if returns.startswith('std::tuple'):
                ret_type = returns[24: -1]
            else:
                ret_type = None
            args = [RX_ARG.fullmatch(x).group('type', 'name') for x in args_str.split(', ')]
            res.append((ret_type, name[3:], args))
    return res


def run():
    commands = parse_commands()
    with open(OUTPUT + '.hpp', 'w') as f:
        f.write(
            '// This file is generated by gen_commands.py, do not edit\n\n'
            '#include "../cm_command_type.hpp"\n'
            'namespace citymania {\n'
            'namespace cmd {\n\n'
        )
        for rt, name, args in commands:
            args_list = ', '.join(f'{at} {an}' for at, an in args)
            args_init = ', '.join(f'{an}{{{an}}}' for _, an in args)
            f.write(
                f'class {name}: public Command {{\n'
                f'public:\n'
            )
            for at, an in args:
                f.write(f'    {at} {an};\n')
            f.write(
                f'\n'
                f'    {name}({args_list})\n'
                f'        :{args_init} {{}}\n'
                f'\n'
                f'    bool Post(bool automatic=False);\n'
                f'}};\n\n'
            )
        f.write(
            '}  // namaespace cmd\n'
            '}  // namaespace citymania\n'
        )

    with open(OUTPUT + '.cpp', 'w') as f:
        f.write(
            '// This file is generated by gen_commands.py, do not edit\n\n'
            '#include "cm_gen_commands.cpp"\n'
            'namespace citymania {\n'
            'namespace cmd {\n\n'
        )
        for rt, name, args in commands:
            constant = 'CMD_' + RX_CAMEL_TO_SNAKE.sub('_', name).upper()
            args_list = ', '.join(f'this->{an}' for _, an in args)
            f.write(
                f'bool {name}::Post(bool automatic) {{\n'
                f'    return ::Command<{constant}>({args_list});\n'
                '}\n\n'
            )
        f.write(
            '}  // namaespace cmd\n'
            '}  // namaespace citymania\n'
        )

if __name__ == "__main__":
    run()