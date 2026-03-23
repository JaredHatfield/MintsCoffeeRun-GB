# Mint's Coffee Run (Game Boy)

A minimal Game Boy project built with [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020).

The game currently boots to a static title screen that displays `Mint's Coffee Run`.

## Build

Install the official GBDK SDK once:

```sh
just sdk-install
```

Then build the ROM:

```sh
just build
```

You can also point the build at an existing GBDK install:

```sh
GBDK_HOME=/path/to/gbdk just build
```

For quick iteration in SameBoy:

```sh
just sameboy
```

The ROM is written to `dist/mints-coffee-run.gb`.
