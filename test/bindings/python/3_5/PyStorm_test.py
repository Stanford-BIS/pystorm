import unittest
import Pystorm as ps
import numpy as np

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

class TestWeights(unittest.TestCase):
    
    def test_constructor(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        self.assertRaises(Exception,ps.Weights,(None,a.shape[0],a.shape[1]))
        self.assertRaises(Exception,ps.Weights,(a,0,a.shape[1]))
        self.assertRaises(Exception,ps.Weights,(a,a.shape[0],0))


    def test_GetNumRows(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertEqual(a.shape[0],w.GetNumRows())

    def test_GetNumColumns(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertEqual(a.shape[1],w.GetNumColumns())

    def test_GetElement(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertRaises(Exception,w.GetElement,(-1,1))
        self.assertRaises(Exception,w.GetElement,(1,-1))
        self.assertRaises(Exception,w.GetElement,(0,a.shape[1]))
        self.assertRaises(Exception,w.GetElement,(a.shape[0],0))

        for i in range(w.GetNumRows()):                                       
            for j in range(w.GetNumColumns()):                                
                self.assertEqual(a[i,j], w.GetElement(i,j))         

    def test_SetElement(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertRaises(Exception,w.SetElement,(-1,1),0)
        self.assertRaises(Exception,w.SetElement,(1,-1),0)
        self.assertRaises(Exception,w.SetElement,(0,a.shape[1],0))
        self.assertRaises(Exception,w.SetElement,(a.shape[0],0,0))

        for i in range(w.GetNumRows()):                                       
            for j in range(w.GetNumColumns()):                                
                new_value = a.item((i,j))+1
                w.SetElement(i,j,new_value)

        for i in range(w.GetNumRows()):                                       
            for j in range(w.GetNumColumns()):                                
                self.assertEqual(a[i,j]+1, w.GetElement(i,j))         

class TestConnection(unittest.TestCase):
    num_neurons = 20
    dims = [3, 2]
    conn_in  = [ps.Input("Input1", dims[0]), ps.Input("Input2", dims[1])]
    conn_out = [ps.Output("Output1", dims[0]), ps.Output("Output2", dims[1])]
    pool     = [ps.Pool("Pool1", num_neurons, dims[0]), 
        ps.Pool("Pool2", num_neurons, dims[1])]
    bucket   = [ps.Bucket("Bucket1", dims[0]), ps.Bucket("Bucket2", dims[1])]
    weights_3_3 = ps.Weights([[1,2,3],
                              [4,5,6],
                              [7,8,9]])
    weights_2_3 = ps.Weights([[1,2],
                              [4,5],
                              [7,8]])
    weights_3_2 = ps.Weights([[1,2,3],
                              [4,5,6]])

    def test_constructor_Input_Pool(self):
        try:
            raised = False
            conn = ps.Connection("conn", self.conn_in[0], self.pool[0], 
                self.weights_3_3)
            conn = ps.Connection("conn", self.conn_in[0], self.pool[1], 
                self.weights_3_2)
            conn = ps.Connection("conn", self.conn_in[1], self.pool[0],     
                self.weights_2_3)
            conn = ps.Connection("conn", self.conn_in[0], self.pool[1])
            conn = ps.Connection("conn", self.conn_in[1], self.pool[0])
        except:
            raised = True
            self.assertFalse(raised, "Exception raised with Input-Pool conn")

        self.assertRaises(Exception, ps.Connection,
            (self.conn_in[1], self.pool[0], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.conn_in[0], self.pool[1], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.conn_in[0], self.pool[1], self.weights_3_2))
        self.assertRaises(Exception, ps.Connection,
            (self.conn_in[0], self.pool[1], self.weights_2_3))
        
    def test_constructor_Pool_Bucket(self):
        try:
            raised = False
            conn = ps.Connection("conn", self.pool[0], self.bucket[0], 
                self.weights_3_3)
            conn = ps.Connection("conn", self.pool[0], self.bucket[1], 
                self.weights_3_2)
            conn = ps.Connection("conn", self.pool[1], self.bucket[0], 
                self.weights_2_3)
            conn = ps.Connection("conn", self.pool[0], self.bucket[1]) 
            conn = ps.Connection("conn", self.pool[1], self.bucket[0]) 
        except:
            raised = True
            self.assertFalse(raised, "Exception raised with Pool-Bucket conn")

        self.assertRaises(Exception, ps.Connection,
            (self.pool[1], self.bucket[0], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.pool[0], self.bucket[1], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.pool[1], self.bucket[0], self.weights_2_3))
        self.assertRaises(Exception, ps.Connection,
            (self.pool[0], self.bucket[1], self.weights_3_2))

    def test_constructor_Bucket_Output(self):
        try:
            raised = False
            conn = ps.Connection("conn", self.bucket[0], self.conn_out[0], 
                self.weights_3_3)
            conn = ps.Connection("conn", self.bucket[0], self.conn_out[1], 
                self.weights_3_2)
            conn = ps.Connection("conn", self.bucket[1], self.conn_out[0], 
                self.weights_2_3)
            conn = ps.Connection("conn", self.bucket[0], self.conn_out[1])
            conn = ps.Connection("conn", self.bucket[1], self.conn_out[0])
        except:
            raised = True
            self.assertFalse(raised, "Exception raised with Bucket-Output conn")

        self.assertRaises(Exception, ps.Connection,
            (self.bucket[1], self.conn_out[0], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.bucket[0], self.conn_out[1], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.bucket[1], self.conn_out[0], self.weights_2_3))
        self.assertRaises(Exception, ps.Connection,
            (self.bucket[0], self.conn_out[1], self.weights_3_2))

    def test_constructor_Bucket_Bucket(self):
        try:
            raised = False
            conn = ps.Connection("conn", self.bucket[0], self.bucket[0], 
                self.weights_3_3)
            conn = ps.Connection("conn", self.bucket[0], self.bucket[1], 
                self.weights_3_2)
            conn = ps.Connection("conn", self.bucket[1], self.bucket[0], 
                self.weights_2_3)
            conn = ps.Connection("conn", self.bucket[0], self.bucket[1])
            conn = ps.Connection("conn", self.bucket[1], self.bucket[0])
        except:
            raised = True
            self.assertFalse(raised, "Exception raised with Bucket-Bucket conn")

        self.assertRaises(Exception, ps.Connection,
            (self.bucket[1], self.bucket[0], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.bucket[0], self.bucket[1], self.weights_3_3))
        self.assertRaises(Exception, ps.Connection,
            (self.bucket[1], self.bucket[0], self.weights_2_3))
        self.assertRaises(Exception, ps.Connection,
            (self.bucket[0], self.bucket[1], self.weights_3_2))

    def test_constructor_Input_Output(self):
        self.assertRaises(Exception, ps.Connection, 
            (self.conn_in[0], self.conn_out[0], self.weights_3_3))

    def test_constructor_Input_Input(self):
        self.assertRaises(Exception, ps.Connection,
            (self.conn_in[0], self.conn_in[0], self.weights_3_3))

    def test_constructor_Output_Output(self):
        self.assertRaises(Exception, ps.Connection,
            (self.conn_out[0], self.conn_out[0], self.weights_3_3))

    def test_constructor_Pool_Pool(self):
        self.assertRaises(Exception, ps.Connection,
            (self.pool[0], self.pool[0], self.weights_3_3))
        
    def test_GetLabel(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0], 
            self.weights_3_3)

        self.assertEqual("conn",conn.GetLabel())

    def test_GetSource(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0], 
            self.weights_3_3)

        self.assertEqual(self.conn_in[0].GetLabel(),
            conn.GetSource().GetLabel())
        self.assertEqual(self.conn_in[0].GetNumDimensions(),
            conn.GetSource().GetNumDimensions())

    def test_GetDest(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0], 
            self.weights_3_3)

        self.assertEqual(self.pool[0].GetLabel(),
            conn.GetDest().GetLabel())
        self.assertEqual(self.pool[0].GetNumDimensions(),
            conn.GetDest().GetNumDimensions())
        self.assertEqual(self.pool[0].GetNumNeurons(),
            conn.GetDest().GetNumNeurons())

    def test_GetWeights(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0], 
            self.weights_3_3)

        weights = conn.GetWeights()

        for i in range(weights.GetNumRows()):
            for j in range(weights.GetNumColumns()):
                self.assertEqual(self.weights_3_3.GetElement(i,j),weights.GetElement(i,j))

    def test_SetWeights(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0])
        weights = conn.GetWeights()

        self.assertEqual(weights,None)

        conn.SetWeights(self.weights_3_3)

        weights = conn.GetWeights()

        for i in range(weights.GetNumRows()):
            for j in range(weights.GetNumColumns()):
                self.assertEqual(self.weights_3_3.GetElement(i,j),weights.GetElement(i,j))

class TestNetwork(unittest.TestCase):
    num_neurons = 100
    dims = 3
    pool_width = 10
    pool_height = 10

    def test_constructor(self):
        try:
            raised = False
            net = ps.Network("net")
        except:
            raised = True
            self.assertFalse(raised, "Exception raised constructing Network")

        self.assertRaises(Exception,ps.Network,(""))

    def test_GetName(self):
        net = ps.Network("net")

        self.assertEqual("net",net.GetName())
        
    def test_CreatePool(self):
        net = ps.Network("net")
        try:
            raised = False
            pool1 = net.CreatePool("pool1", self.num_neurons, self.dims)
            pool2 = net.CreatePool("pool2", self.num_neurons, self.dims, 
                self.pool_width, self.pool_height)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating pool")

    def test_CreateBucket(self):
        net = ps.Network("net")
        try:
            raised = False
            bucket = net.CreateBucket("bucket", self.dims)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating bucket")

    def test_CreateInput(self):
        net = ps.Network("net")

        try:
            raised = False
            conn_in = net.CreateInput("conn_in_1",self.dims)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating input")

    def test_CreateOutput(self):
        net = ps.Network("net")

        try:
            raised = False
            conn_out = net.CreateOutput("conn_out_1",self.dims)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating output")

    def test_CreateConnection(self):
        dims = [3,2]
        conn_obj_labels = ["bucket_in","bucket_out"]
        conn_labels = ["conn_1","conn_2"]

        conn_in = ps.Bucket(conn_obj_labels[0], dims[0])
        conn_out = ps.Bucket(conn_obj_labels[1], dims[1])
        weights = np.array([[1,2,3],
                            [4,5,6]])
        net = ps.Network("net")

        try:
            raised = False
            conn = net.CreateConnection(conn_labels[0],conn_in, conn_out)
            conn = net.CreateConnection(conn_labels[1], conn_in, conn_out, weights)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating connections")

        try:
            raised = False
            conns = net.GetConnections()
            for i,conn in enumerate(conns):
                self.assertEqual(conn_labels[i],conn.GetLabel())
                src  = conn.GetSource()
                dest = conn.GetDest()
                w    = conn.GetWeights()

                self.assertEqual(src.GetNumDimensions(),dims[0])
                self.assertEqual(dest.GetNumDimensions(),dims[1])
                for j in range(w.GetNumRows()):
                    for k in range(w.GetNumColumns()):
                        if (i == 0):
                            self.assertEqual(w.GetElement(i,j),0)
                        else:
                            self.assertEqual(w.GetElement(i,j),weights.item(i,j))
        except:
            raised = True
            self.assertFalse(raised, "Exception raised getting connections")
