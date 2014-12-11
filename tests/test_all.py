import unittest
import sys, os, glob

project_root = os.getcwd()
test_root = os.path.dirname(os.path.abspath(__file__))
test_files = glob.glob(os.path.join(test_root, 'test_*.py'))

os.chdir(test_root)
test_names = [os.path.basename(name)[:-3] for name in test_files]
os.chdir(project_root)

suite = unittest.defaultTestLoader.loadTestsFromNames(test_names)

if __name__ == "__main__":
	runner = unittest.TextTestRunner()
	runner.run(suite)
