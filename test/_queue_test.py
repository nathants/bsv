import os
import shell
import queue
from hypothesis import given, settings
from hypothesis.strategies import composite, integers, sampled_from, randoms
from test_util import run, clone_source

def setup_module(m):
    m.tempdir = clone_source()
    m.orig = os.getcwd()
    m.path = os.environ['PATH']
    os.chdir(m.tempdir)
    os.environ['PATH'] = f'{os.getcwd()}/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/bin'
    shell.run('make clean', stream=True)
    shell.run('make _queue')

def teardown_module():
    with shell.climb_git_root():
        shell.run('make clean', stream=True)

@composite
def inputs(draw):
    capacity = draw(integers(min_value=1, max_value=16))
    num_actions = draw(integers(min_value=0, max_value=256))
    rand = draw(randoms())
    actions = []
    for _ in range(num_actions):
        possible_actions = [
            f'put {rand.randint(0, 999)}',
            'get',
        ]
        actions.append(draw(sampled_from(possible_actions)))
    return capacity, actions

def expected(arg):
    capacity, actions = arg
    res = []
    q = queue.Queue(capacity)
    for action in actions:
        if action == 'get':
            try:
                res.append(q.get_nowait())
            except queue.Empty:
                res.append('empty')
        elif action.split()[0] == 'put':
            try:
                q.put_nowait(action.split()[1])
            except queue.Full:
                res.append('full')
        else:
            assert False, action
    return '\n'.join(map(str, res))

@given(inputs())
@settings(max_examples=100 * int(os.environ.get('TEST_FACTOR', 1)), deadline=os.environ.get("TEST_DEADLINE", 1000 * 60))
def test_props(arg):
    capacity, actions = arg
    result = expected(arg)
    assert result == run('\n'.join(actions) + '\n', '_queue', capacity).strip()
