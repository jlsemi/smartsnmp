import unittest
from smartsnmp_testcase import *

class AgentXv3TestCase(unittest.TestCase, SmartSNMPTestFramework, SmartSNMPTestCase):
	def setUp(self):
		self.agentx_setup("config/agentx.conf")
		self.version = "3"
		self.user = "rwAuthOnlyUser"
		self.level = "authNoPriv"
		self.auth_protocol = "MD5"
		self.auth_key = "rwAuthOnlyUser"
		self.priv_protocol = ""
		self.priv_key = ""
		self.ip = "127.0.0.1"
		self.port = 161
		if self.netsnmp.isalive() == False:
			self.netsnmp.read()
			raise Exception("NetSNMP daemon start error!")
		if self.agentx.isalive() == False:
			self.agentx.read()
			raise Exception("AgentX daemon start error!")

	def tearDown(self):
		if self.netsnmp.isalive() == False:
			self.netsnmp.read()
			raise Exception("NetSNMP daemon start error!")
		if self.agentx.isalive() == False:
			self.agentx.read()
			raise Exception("AgentX daemon start error!")
		self.agentx_teardown()


if __name__ == '__main__':
    unittest.main()
