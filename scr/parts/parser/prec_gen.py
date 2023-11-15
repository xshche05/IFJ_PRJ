d = {
    'unwrap': [1, 'non'],
    'not': [2, 'right'],
    'mul': [3, 'left'],
    # 'div' : [3, 'left'],
    'add': [4, 'left'],
    # 'sub' : [4, 'left'],
    'nil_col': [5, 'right'],
    'eq': [6, 'non'],
    # 'neq' : [6, 'non'],
    # 'gt' : [6, 'non'],
    # 'lt' : [6, 'non'],
    # 'gte' : [6, 'non'],
    # 'lte' : [6, 'non'],
    'and': [7, 'left'],
    'or': [8, 'left'],
    '(': [],
    'i': [],
    ')': [],
    '$': [],
}

operators = ['unwrap',
             'not',
             'mul',
             'div',
             'add',
             'sub',
             'nil_col',
             'eq',
             'neq',
             'gt',
             'lt',
             'gte',
             'lte',
             'and',
             'or']

print(' ', end=',')
for red_y in d.keys():
    print(red_y, end=',')
print()
for yellow_x in d.keys():
    print(yellow_x, end=',')
    for red_y in d.keys():
        # id
        if red_y == 'i':
            if yellow_x in ['not', 'mul', 'add', 'nil_col', 'eq', 'and', 'or', '(', '$']:
                print('<', end=',')
            else:
                print(' ', end=',')
        elif yellow_x == 'i':
            if red_y in ['unwrap', 'mul', 'add', 'nil_col', 'eq', 'and', 'or', ')', '$']:
                print('>', end=',')
            else:
                print(' ', end=',')
        elif red_y == ')' and yellow_x == '(':
            print('=', end=',')
        elif yellow_x == '(' and red_y not in [')', '$']:
            print('<', end=',')
        elif red_y == ')' and yellow_x not in ['(', '$']:
            print('>', end=',')
        elif red_y == '(' and yellow_x in ['mul', 'not', 'add', 'nil_col', 'eq', 'and', 'or', '$']:
            print('<', end=',')
        elif yellow_x == ')' and red_y in ['mul', 'add', 'nil_col', 'eq', 'and', 'or', '$']:
            print('>', end=',')
        elif yellow_x == '$' and red_y in ['mul', 'add', 'nil_col', 'eq', 'and', 'or', 'not', 'unwrap']:
            print('<', end=',')
        elif red_y == '$' and yellow_x in ['mul', 'add', 'nil_col', 'eq', 'and', 'or', 'not', 'unwrap']:
            print('>', end=',')
        # ops
        elif yellow_x in operators and red_y in operators:
            if d[red_y][0] < d[yellow_x][0]:
                print('<', end=',')

            if d[red_y][0] > d[yellow_x][0]:
                print('>', end=',')

            if d[yellow_x][0] == d[red_y][0]:
                if d[yellow_x][1] == 'left' and d[red_y][1] == 'left':
                    print('>', end=',')
                elif d[yellow_x][1] == 'right' and d[red_y][1] == 'right':
                    print('<', end=',')
                else:
                    print(' ', end=',')
        else:
            print(' ', end=',')
    print()