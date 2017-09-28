import unittest
import PyStorm as ps
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

    def test_get_label(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.label, _pool.get_label())

    def test_get_num_neurons(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.num_neurons, _pool.get_num_neurons())

    def test_get_num_dimensions(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.num_dims, _pool.get_num_dimensions())

    def test_get_num_dimensions(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertEqual(self.num_dims, _pool.get_num_dimensions())

    def test_get_width(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertRaises(RuntimeError,_pool.get_width)

        _pool.set_size(self.width,self.height)

        self.assertEqual(_pool.get_width(), self.width)

        _pool2 = ps.Pool(self.label, self.num_neurons, self.num_dims,
            self.width, self.height)

        self.assertEqual(_pool2.get_width(), self.width)

    def test_get_height(self):
        _pool = ps.Pool(self.label, self.num_neurons, self.num_dims)
        self.assertRaises(RuntimeError,_pool.get_height)

        _pool.set_size(self.width,self.height)

        self.assertEqual(_pool.get_height(), self.height)

        _pool2 = ps.Pool(self.label, self.num_neurons, self.num_dims,
            self.width, self.height)

        self.assertEqual(_pool2.get_height(), self.height)

class TestBucket(unittest.TestCase):
    label = "Bucket1"
    num_dims = 3

    def test_constructor(self):
        self.assertRaises(Exception, ps.Bucket)
        self.assertRaises(Exception, ps.Bucket,("",self.num_dims))
        self.assertRaises(Exception, ps.Bucket,(self.label,0))

    def test_get_label(self):
        _bucket = ps.Bucket(self.label, self.num_dims)
        self.assertEqual(self.label, _bucket.get_label())

    def test_get_num_dimensions(self):
        _bucket = ps.Bucket(self.label, self.num_dims)
        self.assertEqual(self.num_dims, _bucket.get_num_dimensions())

class TestInput(unittest.TestCase):
    label = "Input1"
    num_dims = 3

    def test_constructor(self):
        self.assertRaises(Exception, ps.Input)
        self.assertRaises(Exception, ps.Input,("",self.num_dims))
        self.assertRaises(Exception, ps.Input,(self.label,0))

    def test_get_label(self):
        _input = ps.Input(self.label, self.num_dims)
        self.assertEqual(self.label, _input.get_label())

    def test_get_num_dimensions(self):
        _input = ps.Input(self.label, self.num_dims)
        self.assertEqual(self.num_dims, _input.get_num_dimensions())

class TestOutput(unittest.TestCase):
    label = "Output1"
    num_dims = 3

    def test_constructor(self):
        self.assertRaises(Exception, ps.Output)
        self.assertRaises(Exception, ps.Output,("",self.num_dims))
        self.assertRaises(Exception, ps.Output,(self.label,0))

    def test_get_label(self):
        _output = ps.Input(self.label, self.num_dims)
        self.assertEqual(self.label, _output.get_label())

    def test_get_num_dimensions(self):
        _output = ps.Output(self.label, self.num_dims)
        self.assertEqual(self.num_dims, _output.get_num_dimensions())

class TestWeights(unittest.TestCase):

    def test_constructor(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        self.assertRaises(Exception,ps.Weights,(None,a.shape[0],a.shape[1]))
        self.assertRaises(Exception,ps.Weights,(a,0,a.shape[1]))
        self.assertRaises(Exception,ps.Weights,(a,a.shape[0],0))


    def test_get_num_rows(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertEqual(a.shape[0],w.get_num_rows())

    def test_get_num_columns(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertEqual(a.shape[1],w.get_num_columns())

    def test_get_element(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertRaises(Exception,w.get_element,(-1,1))
        self.assertRaises(Exception,w.get_element,(1,-1))
        self.assertRaises(Exception,w.get_element,(0,a.shape[1]))
        self.assertRaises(Exception,w.get_element,(a.shape[0],0))

        for i in range(w.get_num_rows()):
            for j in range(w.get_num_columns()):
                self.assertEqual(a[i,j], w.get_element(i,j))

    def test_set_element(self):
        a = np.array([[1,2,3],
                      [4,5,6]])

        w = ps.Weights(a)

        self.assertRaises(Exception,w.set_element,(-1,1),0)
        self.assertRaises(Exception,w.set_element,(1,-1),0)
        self.assertRaises(Exception,w.set_element,(0,a.shape[1],0))
        self.assertRaises(Exception,w.set_element,(a.shape[0],0,0))

        for i in range(w.get_num_rows()):
            for j in range(w.get_num_columns()):
                new_value = a.item((i,j))+1
                w.set_element(i,j,new_value)

        for i in range(w.get_num_rows()):
            for j in range(w.get_num_columns()):
                self.assertEqual(a[i,j]+1, w.get_element(i,j))

class TestConnection(unittest.TestCase):
    num_neurons = 20
    dims = [3, 2]
    conn_in  = [ps.Input("Input1", dims[0]), ps.Input("Input2", dims[1])]
    conn_out = [ps.Output("Output1", dims[0]), ps.Output("Output2", dims[1])]
    pool     = [ps.Pool("Pool1", num_neurons, dims[0]),
        ps.Pool("Pool2", num_neurons, dims[1])]
    bucket   = [ps.Bucket("Bucket1", dims[0]), ps.Bucket("Bucket2", dims[1])]
    weights_3_3 = ps.Weights(np.array([[1,2,3],
                              [4,5,6],
                              [7,8,9]]))
    weights_2_3 = ps.Weights(np.array([[1,2],
                              [4,5],
                              [7,8]]))
    weights_3_2 = ps.Weights(np.array([[1,2,3],
                              [4,5,6]]))

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

    def test_get_label(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0],
            self.weights_3_3)

        self.assertEqual("conn",conn.get_label())

    def test_get_source(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0],
            self.weights_3_3)

        self.assertEqual(self.conn_in[0].get_label(),
            conn.get_source().get_label())
        self.assertEqual(self.conn_in[0].get_num_dimensions(),
            conn.get_source().get_num_dimensions())

    def test_get_dest(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0],
            self.weights_3_3)

        self.assertEqual(self.pool[0].get_label(),
            conn.get_dest().get_label())
        self.assertEqual(self.pool[0].get_num_dimensions(),
            conn.get_dest().get_num_dimensions())
        self.assertEqual(self.pool[0].get_num_neurons(),
            conn.get_dest().get_num_neurons())

    def test_get_weights(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0],
            self.weights_3_3)

        weights = conn.get_weights()

        for i in range(weights.get_num_rows()):
            for j in range(weights.get_num_columns()):
                self.assertEqual(self.weights_3_3.get_element(i,j),weights.get_element(i,j))

    def test_set_weights(self):
        conn = ps.Connection("conn", self.conn_in[0], self.pool[0])
        weights = conn.get_weights()

        self.assertEqual(weights,None)

        conn.set_weights(self.weights_3_3)

        weights = conn.get_weights()

        for i in range(weights.get_num_rows()):
            for j in range(weights.get_num_columns()):
                self.assertEqual(self.weights_3_3.get_element(i,j),weights.get_element(i,j))

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

    def test_get_name(self):
        net = ps.Network("net")

        self.assertEqual("net",net.get_name())

    def test_create_pool(self):
        net = ps.Network("net")
        try:
            raised = False
            pool1 = net.create_pool("pool1", self.num_neurons, self.dims)
            pool2 = net.create_pool("pool2", self.num_neurons, self.dims,
                self.pool_width, self.pool_height)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating pool")

    def test_create_bucket(self):
        net = ps.Network("net")
        try:
            raised = False
            bucket = net.create_bucket("bucket", self.dims)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating bucket")

    def test_create_input(self):
        net = ps.Network("net")

        try:
            raised = False
            conn_in = net.create_input("conn_in_1",self.dims)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating input")

    def test_create_output(self):
        net = ps.Network("net")

        try:
            raised = False
            conn_out = net.create_output("conn_out_1",self.dims)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating output")

    def test_create_connection_get_connection(self):
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
            conn = net.create_connection(conn_labels[0],conn_in, conn_out)
            conn = net.create_connection(conn_labels[1], conn_in, conn_out, weights)
        except:
            raised = True
            self.assertFalse(raised, "Exception raised creating connections")

        try:
            raised = False
            conns = net.get_connections()
            for i,conn in enumerate(conns):
                self.assertEqual(conn_labels[i],conn.get_label())
                src  = conn.get_source()
                dest = conn.get_dest()
                w    = conn.get_weights()

                self.assertEqual(src.get_num_dimensions(),dims[0])
                self.assertEqual(dest.get_num_dimensions(),dims[1])
                for j in range(w.get_num_rows()):
                    for k in range(w.get_num_columns()):
                        if (i == 0):
                            self.assertEqual(w.get_element(i,j),0)
                        else:
                            self.assertEqual(w.get_element(i,j),weights.item(i,j))
        except:
            raised = True
            self.assertFalse(raised, "Exception raised getting connections")

    def test_get_inputs(self):
        num_inputs = 4
        net = ps.Network("net")
        for i in range(num_inputs):
            net.create_input("in",3)

        inputs = net.get_inputs()
        self.assertEqual(num_inputs,len(inputs))

    def test_get_outputs(self):
        num_outputs = 4
        net = ps.Network("net")
        for i in range(num_outputs):
            net.create_output("out",3)

        outputs = net.get_outputs()
        self.assertEqual(num_outputs,len(outputs))

    def test_get_buckets(self):
        num_buckets = 4
        net = ps.Network("net")
        for i in range(num_buckets):
            net.create_bucket("bucket",3)

        buckets = net.get_buckets()
        self.assertEqual(num_buckets,len(buckets))

    def test_get_pools(self):
        num_pools = 4
        net = ps.Network("net")
        for i in range(num_pools):
            net.create_pool("pool", self.num_neurons, self.dims)

        pools = net.get_pools()
        self.assertEqual(num_pools,len(pools))
