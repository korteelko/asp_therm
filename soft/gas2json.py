import json
import optparse
import sys
from collections import OrderedDict
from os import getcwd, listdir
from os.path import basename, dirname, join, isdir, isfile 

import xml.etree.cElementTree as ET


inp_format = '.xml'


def is_rigth_format(filename):
    return filename.endswith(inp_format)


class InpConfig:
    def __init__(self, options, config_options):
        self.options = options
        self.config_options = config_options

    def is_recursive(self):
        return self.options.is_recursive

    def is_force(self):
        return self.options.is_force

    def recursive_depth(self):
        return self.options.depth
     
    def recursive_depth(self):
        return self.options.depth


class GasXmlNode:
    def __init__(self, name, text):
        self.name = name
        self.text = text
        self.childs = list()

    def add_child(self, child):
        self.childs.append(child)

    def set_text(self, text):
        self.text = text


class GasXmlTree:
    def __init__(self, xml_root):
        self.xml_root = xml_root
        self.gas_root = GasXmlNode(xml_root.attrib['name'], '')
        self.init_subtree(self.xml_root, self.gas_root)
        self.json_string = self.create_json_string()

    def init_subtree(self, node, gas_node):
        # если у узла есть подузлы, обойдём и их
        if len(node):
            for child in node:
                gas_child = GasXmlNode(child.attrib['name'], '')
                gas_node.add_child(gas_child)
                self.init_subtree(child, gas_child)
        # если узел является листом - считываем значение
        else:
            gas_node.set_text(node.text)

    def create_jsonnode_string(self, shift_str, gas_node):
        # как и в инит методе, если это узел обходим рекурсивно
        result_str = '\n'
        if len(gas_node.childs):
            # result_str += shift_str + gas_node.name + ': {\n'
            result_str += '{0}"{1}":\n'.format(shift_str, gas_node.name) + shift_str + '{\n'
            for child in gas_node.childs:
                result_str += self.create_jsonnode_string(shift_str + '\t', child) + ','
            # для последнего дочернего узла убрать запятую за ним, добавить перевод строки
            result_str = result_str[:-1] + '\n'
            result_str += shift_str + '}'
        else:
            result_str += '{0}"{1}": "{2}"'.format(shift_str, gas_node.name, gas_node.text)
        return result_str

    def create_json_string(self):
        # строка отступов
        result_str = '{'
        result_str += self.create_jsonnode_string('\t', self.gas_root)
        result_str +='\n}\n'
        return result_str


    def get_json_string(self):
        return self.json_string

def xml2json(filename, is_force):
    xname = basename(filename)
    xdir = dirname(filename)
    tree = ET.parse(filename)
    root = tree.getroot()
    gas_tree = GasXmlTree(root)
    print(gas_tree.get_json_string())
    #if not is_force:
    #    try:
    #        ans = input('Продолжить работу с выбранными файлами')
    #    asd



def add_directory(dirname, is_recursive, rdepth):
    if rdepth <= 0:
        return list()
    print('Сканируем директорию: ' + dirname)
    files = [f for f in listdir('./') if isfile(join('./', f)) and is_rigth_format(f)]
    for f in files:
        print('  Добавляем файл: ' + f)
    if is_recursive:
        dirs = [f for f in listdir('./') if isdir(join('./', f))]
        files.extend([add_directory(d) for (d) in dirs])
    return files


def parse_input(inputnames, inp_config):
    filenames = list()
    for inp in inputnames:
        # ???? maybe it is wrong
        # print('debug: ' + inp)
        inp = join(getcwd(), inp)
        if isfile(inp):
            if is_rigth_format(inp):
                print('Добавляем файл: ' + inp)
                filenames.append(inp)
        elif isdir(inp):
            filenames.extend(add_directory(inp, inp_config.is_recursive(), inp_config.recursive_depth()))
        else:
            print('Не пойму, что это - ' + inp)
    # process xmlfiles
    #   здеся вывод всего и спросить ок или не ок
    print('Конвертируем файлы:')
    for f in filenames:
        print(f)
    for filename in filenames:
        if isfile(filename):
            xml2json(filename, inp_config.is_force())
        else:
            print('  Файл: ' + filename + ' проигнорирован')


# на вход имя(имена) файлов для преобразования
def main():
    params = optparse.OptionParser(
            description='Конвертировать xml файл с описанием какого-либо газового компонента в json формат',
            usage='gas2json [files or dirs]')
    params.add_option(
            '--recursive', '-r', action='store_true',
            dest='is_recursive', help='Обход вложенных директорий')
    params.add_option(
            '--force', '-f', action='store_true',
            dest='is_force', help='Переписыть выходные файлы без подтверждения')
    params.add_option(
            '--depth', '-d', help='Глубина обхода вложенных директорий. По умолчанию - 2', default='2')
    # options - список опций
    # arguments - список файлов, директорий
    options, arguments = params.parse_args()
    try:
        parse_input(arguments, InpConfig(options, list()))
    except Exception as e:
        print('Error occurred while file parsing:')
        print(e)
        print('\nHelp:')
        params.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
