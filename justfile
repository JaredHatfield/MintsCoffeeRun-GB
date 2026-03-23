set shell := ["zsh", "-euc"]

project := "mints-coffee-run"
gbdk_home := if env_var_or_default("GBDK_HOME", "") != "" { env_var_or_default("GBDK_HOME", "") } else { justfile_directory() + "/tools/gbdk" }
lcc := gbdk_home + "/bin/lcc"
rom := justfile_directory() + "/dist/" + project + ".gb"
build_artifacts := "dist/*.gb dist/*.ihx dist/*.map dist/*.noi dist/*.sym dist/*.lst dist/*.cdb dist/*.adb dist/*.asm dist/*.rst"

default:
  @just --list

build: sprites
  mkdir -p dist
  xattr -dr com.apple.quarantine "{{gbdk_home}}" 2>/dev/null || true
  test -x "{{lcc}}" || { echo "Missing GBDK SDK at {{gbdk_home}}"; echo "Run: just sdk-install"; echo "Or set GBDK_HOME to an installed GBDK SDK root."; exit 1; }
  "{{lcc}}" -o "{{rom}}" src/main.c src/sprites/mint_sprite.c src/sprites/mint_title_sprite.c

sprites:
  python3 scripts/piskel_to_gbdk_sprite.py src/sprites/mint.piskel src/sprites/mint_sprite
  python3 scripts/piskel_to_gbdk_sprite.py src/sprites/mint.piskel src/sprites/mint_title_sprite --scale 3

test: build

sdk-install:
  @mkdir -p "{{justfile_directory()}}/tools"; \
  case "$(uname -s)-$(uname -m)" in \
    Darwin-arm64) asset="gbdk-macos-arm64.tar.gz" ;; \
    Darwin-x86_64) asset="gbdk-macos.tar.gz" ;; \
    Linux-x86_64) asset="gbdk-linux64.tar.gz" ;; \
    Linux-aarch64|Linux-arm64) asset="gbdk-linux-arm64.tar.gz" ;; \
    *) echo "Unsupported platform: $(uname -s)-$(uname -m)"; exit 1 ;; \
  esac; \
  tmp="$(mktemp -d)"; \
  curl -fL "https://github.com/gbdk-2020/gbdk-2020/releases/latest/download/${asset}" -o "$tmp/gbdk.tar.gz"; \
  tar -xzf "$tmp/gbdk.tar.gz" -C "$tmp"; \
  rm -rf "{{gbdk_home}}"; \
  mv "$tmp/gbdk" "{{gbdk_home}}"; \
  rm -rf "$tmp"; \
  xattr -dr com.apple.quarantine "{{gbdk_home}}" 2>/dev/null || true

sameboy: build
  open -a SameBoy "{{rom}}"

clean:
  rm -f {{build_artifacts}}
