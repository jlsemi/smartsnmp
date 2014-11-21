#!/bin/sh

export NET_SNMP_VERSION=5.7.2.1
export NET_SNMP_SRC_URL=https://sourceforge.net/projects/net-snmp/files/net-snmp/${NET_SNMP_VERSION}/net-snmp-${NET_SNMP_VERSION}.tar.gz
export NET_SNMP_ROOT_DIR=`pwd`/tests

# Tarball
echo Downloading Net-SNMP ${NET_SNMP_VERSION}
if [ -f ${NET_SNMP_ROOT_DIR}/net-snmp-${NET_SNMP_VERSION}.tar.gz ]; then
  echo "Tarball has been downloaded"
else
  wget -P${NET_SNMP_ROOT_DIR} ${NET_SNMP_SRC_URL}
fi

# Directory
mkdir -p ${NET_SNMP_ROOT_DIR}/net-snmp-src
mkdir -p ${NET_SNMP_ROOT_DIR}/net-snmp-release
tar xzvf ${NET_SNMP_ROOT_DIR}/net-snmp-${NET_SNMP_VERSION}.tar.gz -C ${NET_SNMP_ROOT_DIR}/net-snmp-src
cd ${NET_SNMP_ROOT_DIR}/net-snmp-src/net-snmp-${NET_SNMP_VERSION}

# Configure, make and install
./configure --with-default-snmp-version="3" --with-sys-contact="contact" --with-sys-location="location" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp" --prefix=${NET_SNMP_ROOT_DIR}/net-snmp-release --disable-applications --disable-manuals --disable-scripts --disable-mibs --disable-embedded-perl --disable-deprecated --without-perl-modules --with-out-mib-modules="mibII ucd_snmp agent_mibs notification notification-log-mib target utilities disman/event disman/schedule host" --with-mib_modules="mibII/vacm_vars"
make
make install
