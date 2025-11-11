def complexity_target(value, items, flag):
    total = 0

    if value > 10:
        total += 1
    elif value == 5:
        total += 2
    else:
        total += 3

    while total < 100:
        total += len(items)

    for item in items:
        total += item if item % 2 == 0 else 1

    try:
        risky_call()
    except ValueError:
        total += 5
    except Exception:
        total += 6
    finally:
        total += 7

    match total % 3:
        case 0:
            total += 10
        case 1:
            total += 20
        case _:
            total += 30

    assert flag

    return total


def params_target(a, b, /, c, d=1, *, e, f=2, *args, **kwargs):
    return (a, b, c, d, e, f, args, kwargs)

