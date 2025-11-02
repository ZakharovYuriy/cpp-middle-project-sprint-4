class DemoProcessor:
    """Processor with multiple public methods."""

    def __init__(self, multiplier: float, offset: float):
        self.multiplier = multiplier
        self.offset = offset

    def process(self, data):
        """Inline docstring to be ignored by metrics"""
        transformed = [x * self.multiplier for x in data]
        return [x + self.offset for x in transformed]


def helper_function():
    """Top level helper with docstring that should be ignored."""
    result = []
    # regular comment
    for value in range(3):
        result.append(value)

    return result
