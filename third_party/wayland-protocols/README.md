# wayland-protocols

Protocol XML files copied from the upstream wayland-protocols repository. The
build runs the system `wayland-scanner` on these files and does not read the
system wayland-protocols package, so every machine generates code from the
same XML.

The layout matches upstream: a file at `stable/xdg-shell/xdg-shell.xml` here
is at the same path in the upstream repository.

| File | Upstream tag | Upstream commit | SHA-256 |
| --- | --- | --- | --- |
| `stable/xdg-shell/xdg-shell.xml` | `1.44` | `810f1adaf33521cc55fc510566efba2a1418174f` | `9d9e27038ca722f159a361293f8c6cd270dab907ab52de5b8ecfbb8d6b9651c2` |

Source: <https://gitlab.freedesktop.org/wayland/wayland-protocols>

Tag 1.44 matches the `wayland-protocols` package in Debian trixie, which
`docker/Dockerfile` installs in the `linux` image. The upstream file and the
Debian file have the same SHA-256.

## License

MIT. Each XML file carries its copyright and permission notice in its
`<copyright>` element. Do not remove that element.

## Updating a file

From this directory, copy the file from the new upstream tag, then update the
tag, commit, and SHA-256 in the table above:

```bash
curl -fL -o stable/xdg-shell/xdg-shell.xml \
  https://gitlab.freedesktop.org/wayland/wayland-protocols/-/raw/<tag>/stable/xdg-shell/xdg-shell.xml
git ls-remote https://gitlab.freedesktop.org/wayland/wayland-protocols.git 'refs/tags/<tag>^{}'
sha256sum stable/xdg-shell/xdg-shell.xml
```

Do not edit the XML by hand. The next build regenerates the C header and
source because the XML is a dependency of the generation rule.
