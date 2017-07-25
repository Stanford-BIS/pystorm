import unittest
import Pystorm as ps

class TestPool(unittest.TestCase):
    label = "Pool1"
    num_neurons = 100
    num_dims = 3
    width = 10
    height = 10
    min_neurons = 4
    max_neurons = 4096

    def test_constructor(self):
        self.assertRaises(Exception, ps.Pool)
        self.assertRaises(Exception, ps.Pool,("",self.num_neurons, self.num_dims))
        self.assertRaises(Exception, ps.Pool,(self.label,self.min_neurons-1, self.num_dims))
        self.assertRaises(Exception, ps.Pool,(self.label,self.max_neurons+1, self.num_dims))

    def test_GetLabel(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.label, _pool.GetLabel())

    def test_GetNumNeurons(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.num_neurons, _pool.GetNumNeurons())

    def test_GetNumDimensions(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.num_dims, _pool.GetNumDimensions())

    def test_GetNumDimensions(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.num_dims, _pool.GetNumDimensions())

    def test_GetWidth(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertRaises(RuntimeError,_pool.GetWidth)

        _pool.SetSize(self.width,self.height)

        self.assertEqual(_pool.GetWidth(), self.width)

        _pool2 = ps.Pool(self.label, self.num_neurons, self.num_dims, 
            self.width, self.height)

        self.assertEqual(_pool2.GetWidth(), self.width)

    def test_GetHeight(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertRaises(RuntimeError,_pool.GetHeight)

        _pool.SetSize(self.width,self.height)

        self.assertEqual(_pool.GetHeight(), self.height)

        _pool2 = ps.Pool(self.label, self.num_neurons, self.num_dims, 
            self.width, self.height)

        self.assertEqual(_pool2.GetHeight(), self.height)

class TestBucket(unittest.TestCase):
    label = "Bucket1"
    num_dims = 3

    def test_constructor(self):
        self.assertRaises(Exception, ps.Bucket)
        self.assertRaises(Exception, ps.Bucket,("",self.num_dims))
        self.assertRaises(Exception, ps.Bucket,(self.label,0))

    def test_GetLabel(self):
        _bucket = ps.Bucket(self.label, self.num_dims)
        self.assertEqual(self.label, _bucket.GetLabel())

    def test_GetNumDimensions(self):
        _bucket = ps.Bucket(self.label, self.num_dims)
        self.assertEqual(self.num_dims, _bucket.GetNumDimensions())

class TestInput(unittest.TestCase):
    label = "Input1"
    num_dims = 3

    def test_constructor(self):
        self.assertRaises(Exception, ps.Input)
        self.assertRaises(Exception, ps.Input,("",self.num_dims))
        self.assertRaises(Exception, ps.Input,(self.label,0))
    
    def test_GetLabel(self):
        _input = ps.Input(self.label, self.num_dims)
        self.assertEqual(self.label, _input.GetLabel())

    def test_GetNumDimensions(self):
        _input = ps.Input(self.label, self.num_dims)
        self.assertEqual(self.num_dims, _input.GetNumDimensions())

class TestOutput(unittest.TestCase):
    label = "Output1"
    num_dims = 3

    def test_constructor(self):
        self.assertRaises(Exception, ps.Output)
        self.assertRaises(Exception, ps.Output,("",self.num_dims))
        self.assertRaises(Exception, ps.Output,(self.label,0))
    
    def test_GetLabel(self):
        _output = ps.Input(self.label, self.num_dims)
        self.assertEqual(self.label, _output.GetLabel())

    def test_GetNumDimensions(self):
        _output = ps.Output(self.label, self.num_dims)
        self.assertEqual(self.num_dims, _output.GetNumDimensions())
