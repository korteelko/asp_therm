
# скрипт генерации html представления покрытия кода тестами
# опции там прописаны, но по умолчанию принимает данные проекта

import optparse
import os
import subprocess
import sys


class InpConfig:
    def __init__(self, options):
        self.options = options

    def GetSourceRoot(self):
        return self.options.source_root

    def GetObjectsRoot(self):
        return self.options.objects_root


def parse_input(arguments, inp_config):
    output_dir = str(arguments[0])
    if not output_dir:
        raise Exception('Не задана директория вывода')
    if not os.path.exists(output_dir) or not os.path.isdir(output_dir):
        print('Директория ' + output_dir + ' не существует.')
        print('Попробую создать')
        os.mkdir(output_path)
    source_root = inp_config.GetSourceRoot()
    objects_root = inp_config.GetObjectsRoot()
    # set directory of script as current dir 
    if not source_root and not objects_root:
        source_root = os.path.join(os.path.dirname(os.path.realpath(__file__)), '../source')
        objects_root = os.path.join(os.path.dirname(os.path.realpath(__file__)), '../build')
    if not os.path.exists(source_root) or not os.path.isdir(source_root):
        raise Exception('Путь к исходникам: ' + source_root + ' не валиден')
    if not os.path.exists(objects_root) or not os.path.isdir(objects_root):
        raise Exception('Путь к объектным файлам: ' + objects_root + ' не валиден')
    # проверки закончены, запустим gcovr
    index_file = os.path.join(output_dir, 'index.html')
    # test
    if (0):
        print('В результате строка запуска gcovr имеет вид:')
        print('gcovr -r ' + source_root + objects_root + ' --html --html-details -o ' + index_file)
    #release
    else:
        print('Запускаю gcovr.\nНе забудьте запустить тесты перед использованием!')
        out = subprocess.Popen(['gcovr', '-r', str(source_root), str(objects_root), '--html', '--html-details', '-o', str(index_file)])
        stdout, stderr = out.communicate()
        print('gcovr out:')
        print(stdout)
        print('gcovr err:')
        print(stderr)


def main():
    params = optparse.OptionParser(
            description='Скрипт генерации html отчёта покрытия тестов',
            usage='gcov_report -r source_root -o objects_root html_dir')
    # source root  - ../source
    params.add_option(
            '--source_root', '-r', action='store', type='string',
            dest='source_root', help='Корневая директория тестируемых исходников')
    # objects root - ../build
    params.add_option(
            '--objects_root', '-o', action='store', type='string',
            dest='objects_root', help='Корневая директория объектных файлов, файлов gcov(директория сборки)')
    options, arguments = params.parse_args()
    try:
        parse_input(arguments, InpConfig(options))
    except Exception as e:
        print('Ошибка выполнения:')
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
