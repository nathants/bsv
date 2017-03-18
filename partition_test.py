import pytest
import string
from hypothesis import given, settings
from hypothesis.strategies import text, lists, composite, integers
import shell
import hashlib

stdinpath = '/tmp/%s.stdin' % hashlib.md5(__file__.encode('ascii')).hexdigest()
stdoutpath = '/tmp/%s.stdout' % hashlib.md5(__file__.encode('ascii')).hexdigest()

def unindent(text):
    return '\n'.join([x.lstrip() for x in text.splitlines()]) + '\n'

def run(stdin, *args):
    with open(stdinpath, 'w') as f:
        f.write(unindent(stdin))
    try:
        shell.run(*(('cat', stdinpath, '|') + args + ('>', stdoutpath)), stream=True)
    except:
        raise AssertionError from None
    with open(stdoutpath) as f:
        return f.read()

shell.run('make clean && make partition', stream=True)

# MAX_COLUMNS = 64
# MAX_LINE_BYTES = 8192

# @composite
# def inputs(draw):
#     num_columns = draw(integers(min_value=1, max_value=12))
#     column = text(string.ascii_lowercase)
#     line = lists(column, min_size=num_columns, max_size=num_columns)
#     lines = draw(lists(line))
#     csv = '\n'.join([','.join(x) for x in lines]) + '\n'
#     field = integers(min_value=1, max_value=num_columns)
#     fields = draw(lists(field, min_size=1, max_size=num_columns))
#     fields = ','.join(map(str, fields))
#     return (fields, csv)

# @composite
# def inputs_ascending_unique_fields(draw):
#     fields, csv = draw(inputs())
#     fields = ','.join(sorted(set(fields.split(',')), key=int))
#     return (fields, csv)

# def expected(fields, csv):
#     fields = [int(x) - 1 for x in fields.split(',')]
#     result = []
#     for line in csv.splitlines():
#         if not line.strip():
#             result.append('')
#         else:
#             columns = line.split(',')
#             if len(columns) > 64:
#                 return
#             res = []
#             for field in fields:
#                 if field > 64:
#                     return
#                 try:
#                     res.append(columns[field])
#                 except IndexError:
#                     pass
#             result.append(','.join(res))
#     return '\n'.join(result) + '\n'

# @given(inputs())
# @settings(max_examples=100)
# def test_props(args):
#     fields, csv = args
#     result = expected(fields, csv)
#     if result:
#         assert result == run(csv, './rcut ,', fields)
#     else:
#         with pytest.raises(AssertionError):
#             run(csv, './rcut ,', fields)


def test_basic():
    shell.run('rm -f tmp.*')
    try:
        stdin = """
        0,b,c,d
        1,e,f,g
        2,h,i,j
        """
        assert '' == run(stdin, './partition , 10 tmp')
        data = """
        tmp.00:b,c,d
        tmp.01:e,f,g
        tmp.02:h,i,j
        """
        assert unindent(data).strip() == shell.run('grep ".*" tmp.*')
    finally:
        shell.run('rm -f tmp.*')
