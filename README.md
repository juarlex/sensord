sensord
=======

Daemon to log the data from different sensors into a SQLite3 database, working with Linux in a Raspberry Pi.

## Supported sensors

  * DS18B20 Temperature sensor

## Requirements

  * Packages:

        $ apt-get install build-essential sqlite3 libsqlite3-dev

  * User account:

        $ useradd _sensord -d /var/run/sensord -s /usr/sbin/nologin 

  * Linux kernel with support for DS18B20 sensors:
   
        http://www.picymru.com/633

  * Notes:
      Run all the commands as root.

## How to build and install
        $ make build
        $ make install

## How to create your database

        $ mkdir /var/lib/sensord
        $ cat db/schema.sql | sqlite3 /var/lib/sensord/sensord.sqlite3
        $ chown _sensord /var/lib/sensord/sensord.sqlite3

## How to seed your database

Before to run this command edit the file db/seed.sql and change the path for your device.
Run these commands as root:

        $ cat db/seed.sql | sqlite3 /var/lib/sensord/sensord.sqlite3

## How to start the daemon

        $ /usr/local/sbin/sensord

## Running the daemon without the default settings

        $ /usr/local/sbin/sensord [-i interval_in_seconds_to_check_sensors_status] [-d /path/db] [-u non-root-user]

## LICENSE

This code is released under the GPL version 2, see the COPYING file.
