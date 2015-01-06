import unittest
from smartsnmp_testcase import *

class SNMPv2cTestCase(unittest.TestCase, SmartSNMPTestFramework, SmartSNMPTestCase):
	def setUp(self):
		self.snmp_setup("config/snmp.conf")
		self.version = "2c"
		self.community = "private"
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

if __name__ == '__main__':
    unittest.main()
