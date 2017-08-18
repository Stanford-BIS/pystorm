import unittest
import numpy as np
import Pystorm as ps
from neuromorph.Resources import Source, Sink, MMWeights, AMBuckets, Neurons
from neuromorph.netobjnode import NetObjNode 


class Test_netobjnode(unittest.TestCase):
    def test_create_resources_for_Input(self):
        net_obj_name = "Input"
        net_obj_dims = 3
       
        inp = ps.Input(net_obj_name, net_obj_dims)
        n = NetObjNode(inp)

        self.assertIsInstance(n.get_resource(), Source)
        self.assertEqual(n.get_net_obj(), inp)
        self.assertEqual(n.get_resource().D, net_obj_dims)

    def test_create_resources_for_Output(self):
        net_obj_name = "Output"
        net_obj_dims = 3
        
        out = ps.Output(net_obj_name, net_obj_dims)
        n = NetObjNode(out)

        self.assertIsInstance(n.get_resource(), Sink)
        self.assertEqual(n.get_net_obj(), out)
        self.assertEqual(n.get_resource().D, net_obj_dims)

    def test_create_resources_for_Pool(self):
        net_obj_name = "Pool"
        net_obj_nrn_count = 30
        net_obj_dims = 3
        
        pool = ps.Pool(net_obj_name, net_obj_nrn_count, net_obj_dims)
        n = NetObjNode(pool)

        self.assertIsInstance(n.get_resource(), Neurons)
        self.assertEqual(n.get_net_obj(), pool)
        self.assertEqual(n.get_resource().N, net_obj_nrn_count)

    def test_create_resources_for_Bucket(self):
        net_obj_name = "Bucket"
        net_obj_dims = 3
        
        bkt = ps.Bucket(net_obj_name, net_obj_dims)
        n = NetObjNode(bkt)

        self.assertIsInstance(n.get_resource(), AMBuckets)
        self.assertEqual(n.get_net_obj(), bkt)
        self.assertEqual(n.get_resource().D, net_obj_dims)

    def test_create_resources_for_Weights(self):
        net_obj_name = "Weights"
        arr_rows = 3
        arr_cols = 2
        wt_arr = np.ones((2,3),dtype=np.uint32)
        wt = ps.Weights(wt_arr)
        n = NetObjNode(wt)

        self.assertIsInstance(n.get_resource(), MMWeights)
        self.assertEqual(n.get_net_obj(), wt)

        mmw = n.get_resource().user_W
        for i in range(mmw.shape[0]):
            for j in range(mmw.shape[1]):
                self.assertEqual(mmw[i,j], wt_arr[i,j])

    def test_create_resources_for_Unknown_Type(self):
        net_obj_name = "Network"

        self.assertRaises(TypeError, NetObjNode.__init__, ps.Network(net_obj_name))

    def test_add_adjacent_node(self):
        # test get_adjacent_nodes before and after insserting
        pass
