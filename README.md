WordPress Backup Tool
=====================
This is a simple tool that can be used for automated backup of contents of your WordPress installation. All it does is to log into WordPress administration and download `WXR` (WordPress eXtended RSS) feed containing all of your posts, pages, comments, custom fields, terms, navigation menus, and custom posts.

Usage
-----
```
Usage: wp-backup [options]

Options:
  -h, --help          shows this help and exits
  -v, --version       shows version number and exits
  -u, --username      username for login into WordPress
  -w, --wpurl         URL of the WordPress installation (without "wp-admin" or "wp-login")
  -o, --output-file   destination file name for downloaded XML (default: "wordpress.xml")

  wp-backup reads password from standard input or environmental variable WPPASS.
  WordPress must be v2.5.0 or higher!
```
Basically, there are two ways how the tool can get a password. If there is environmental variable `$WPPASS` set, its contents shall be considered to be your password into WordPress. If the variable is empty, contents of the standard input shall be read.

```bash
echo "secret" | wp-backup -u admin -w http://localhost/wordpress -o save/it/here.xml

# or

export WPPASS="secret"
wp-backup -u admin -w http://localhost/wordpress -o save/it/here.xml
unset WPPASS

```

## License
This program is licensed under GPLv2 license.
