class DemoProcessor:
    """Processor with multiple public methods."""

    def __init__(self, multiplier: float, offset: float):
        self.multiplier = multiplier
        self.offset = offset

    def process(self, data):
        """Inline docstring should also count as code"""
        transformed = [x * self.multiplier for x in data]
        return [x + self.offset for x in transformed]


def helper_function():
    """Top level helper with docstring that counts toward metrics."""
    result = []
    # regular comment
    for value in range(3):
        result.append(value)

    return result
