# uniqueid

Display the system unique ID as a QR code.

## Usage

```
uniqueid [qrcode invert]
```

## Description

- Retrieves the system unique ID and renders it as an ASCII QR code
- `invert`: non-zero value inverts colors (default 1), zero uses normal colors

## Example

```bash
# Display QR code
uniqueid

# Normal colors
uniqueid 0
```
