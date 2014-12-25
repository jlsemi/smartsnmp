import unittest
from smartsnmp_testcmd import *

class SNMPv3TestCase(unittest.TestCase, SmartSNMPTestCmd):
	def setUp(self):
		self.snmp_setup("config/snmp.conf")
		self.version = "3"
		self.user = "noAuthUser"
		self.level = "noAuthNoPriv"
		self.ip = "127.0.0.1"
		self.port = 161

	def tearDown(self):
		self.snmp_teardown()

	@snmp_before_check
	def test_snmpv3get(self):
		self.snmpget_expect(".", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".0", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.3", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.4", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2.1", Oid(".1.3.6.1.2.1.4"), version = '3')
		self.snmpget_expects(((".1.3.6.1.2.1.1.9.1.0", SNMPNoSuchObject()), (".1.3.6.1.2.1.1.2.0", Oid(".1.3.6.1.2.1.1"))), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.1", SNMPNoSuchInstance(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2", SNMPNoSuchInstance(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.5", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.0", SNMPNoSuchObject(), version = '3')

		# a correct request for MIB-II/UDP should be .1.3.6.1.2.1.7.5.1.2.4.x.x.x.x.p
		self.snmpget_expect(".1.3.6.1.2.1.7.5.1.2.4", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.7.5.1.2.4.1.1", SNMPNoSuchInstance())

	@snmp_before_check
	def test_snmpv3getnext(self):
		self.snmpgetnext_expect(".", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"), version = '3')
		self.snmpgetnext_expect(".0", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"), version = '3')
		self.snmpgetnext_expect(".1.3", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"), version = '3')
		self.snmpgetnext_expect(".1.4", ".1.4", SNMPEndOfMib(), version = '3')
		self.snmpgetnext_expect(".1.5.6.7.8.100", ".1.5.6.7.8.100", SNMPEndOfMib(), version = '3')

	@snmp_before_check
	def test_snmpv3set(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess(), version = '3')
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType(), version = '3')
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888), version = '3')
		self.snmpset_expect(".1.3.6.1.2.1.4.0", Integer(8888), SNMPNotWritable(), version = '3')

	@snmp_before_check
	def test_snmpv3walk(self):
		self.snmpwalk_expect(".", version = '3')


if __name__ == '__main__':
    unittest.main()
