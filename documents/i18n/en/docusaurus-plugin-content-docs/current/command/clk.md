# clk

Export clock information in tree format.

## Usage

```
clk [name ...]
```

## Description

- Without arguments, exports the entire clock tree hierarchy
- When a name is specified, only shows that clock and its child nodes
- Each line displays: name, frequency (MHz, 6 decimal places), enable state (1/0)
- Child nodes are indented under their parent node

## Examples

```bash
# Export full clock tree
clk

# View a specific clock
clk uclk
```
