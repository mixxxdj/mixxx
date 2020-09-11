
# Key Rotation

# downloads-hostgator.mixxx.org

SSH access is granted via an RSA key stored in `build/certificates/downloads-hostgator.mixxx.org.key`.

To rotate this key, generate a new RSA key with a strong password (e.g. 32 character randomly generated).

```
ssh-keygen -t rsa -b 4096 -f downloads-hostgator.mixxx.org.key
```

Copy the **public** key to `$HOME/.ssh/authorized_keys`, replacing the old file to remove access for the current key.

Encrypt the password using `travis encrypt` and update `.travis.yml`.

```
travis encrypt DOWNLOADS_HOSTGATOR_DOT_MIXXX_DOT_ORG_KEY_PASSWORD=hunter2 -r mixxxdj/mixxx
```
