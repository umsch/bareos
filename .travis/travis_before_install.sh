#!/bin/bash

print_header()
{
   TEXT="$1"
   printf "#\n"
   printf "# %s\n" "$TEXT"
   printf "#\n"
}

if [ "${COVERITY_SCAN}" ]; then
   cd core
   # run configure with default options
   debian/rules override_dh_auto_configure
   eval "$COVERITY_SCAN_BUILD"
else
   cd regress
   echo '
MAKEOPT=-j4
BAREOS_SOURCE="`pwd`/../core"
EMAIL=my-name@domain.com
SMTP_HOST="localhost"
DBTYPE="postgresql"
WHICHDB=-D${DBTYPE}=yes
TAPE_DRIVE="/dev/null"
AUTOCHANGER="/dev/null"
DRIVE1=0
DRIVE2="none"
SLOT1=1
SLOT2=2
TAPE_DRIVE1="/dev/null"
AUTOCHANGER_SCRIPT=mtx-changer
db_name="regress"
db_user="regress"
db_password=""
TCPWRAPPERS=-Dtcp-wrappers=yes
OPENSSL=-Dopenssl=yes
HOST="127.0.0.1"
SCSICRYPTO="-Dscsi-crypto=yes"
BASEPORT=8101
SITE_NAME=travis-bareos-${HOST}
DEVELOPER=-Ddeveloper=yes
COVERAGE=-Dcoverage=yes
' > config

fi
