import unittest
from smartsnmp_testcase import *

class AgentXv3TestCase(unittest.TestCase, SmartSNMPTestFramework, SmartSNMPTestCase):
	def setUp(self):
		self.agentx_setup("config/agentx.conf")
		self.version = "3"
		self.user = "rwNoAuthUser"
		self.level = "noAuthNoPriv"
		self.auth_protocol = ""
		self.auth_key = ""
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
