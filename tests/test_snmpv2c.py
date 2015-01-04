import unittest
from smartsnmp_testcase import *

class SNMPv2cTestCase(unittest.TestCase, SmartSNMPTestCmd, SmartSNMPCommonCase):
	def setUp(self):
		self.snmp_setup("config/snmp.conf")
		self.version = "2c"
		self.community = "public"
		self.ip = "127.0.0.1"
		self.port = 161
		if self.snmp.isalive() == False:
			self.snmp.read()
			raise Exception("SNMP daemon start error!")

	def tearDown(self):
		if self.snmp.isalive() == False:
			self.snmp.read()
			raise Exception("SNMP daemon start error!")
		self.snmp_teardown()

	def test_snmpv2cset(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPAuthErr(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPAuthErr(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), SNMPAuthErr(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess(), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType(), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.0", Integer(8888), SNMPNotWritable(), community = "private")

if __name__ == '__main__':
    unittest.main()
