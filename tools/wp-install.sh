#!/bin/sh
set -e

version=4.5

archive="$version-branch.zip"
archive_url="https://github.com/WordPress/WordPress/archive/$archive"
installation_dir="/var/www/html/wordpress"
tmp_dir="/tmp/wpbackup-test"

mysql_user=root


rm -rf "$tmp_dir"
mkdir -p "$tmp_dir"

if ! test -f "$tmp_dir/$archive"
then
	echo "Downloading $archive_url..."
	wget --no-check-certificate "$archive_url" -O "$tmp_dir/$archive" || true
	if ! test -f "$tmp_dir/$archive"
	then
		echo "failed."
		exit 1
	fi
fi

if ! test -f "$installation_dir/index.php"
then
	if ! test -d "$tmp_dir/WordPress-$version-branch"
	then
		unzip "$tmp_dir/$archive" -d "$tmp_dir"
	fi
	mv "$tmp_dir/WordPress-$version-branch" "$installation_dir"
fi


db_name="wordpress_$(echo $version | tr \. _)"
mysql -u "$mysql_user" -e "CREATE DATABASE IF NOT EXISTS $db_name;"


wp_url="http://localhost/wordpress-$version"

if ! test -f "$installation_dir/wp-config.php"
then
	install_url="$wp_url/wp-admin/setup-config.php?step=2"
	post="dbname=$db_name&uname=$mysql_user&prefix=wp_&pwd=&dbhost=localhost"
	post="$post&language=en_GB&submit=1"
	curl --location --data "$post" "$install_url" >dev/null
fi

install_url="$wp_url/wp-admin/install.php?step=2"
post="weblog_title=WordPress&user_name=admin&admin_password=secret"
post="$post&admin_password2=secret&pw_weak=1&admin_email=admin@example.com"
post="$post&submit=1"
curl --location "$install_url" --data "$post" >/dev/null

echo "done"
