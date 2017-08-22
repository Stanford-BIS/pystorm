import unittest
import numpy as np
import Pystorm as ps
from neuromorph.Resources import Source, Sink, MMWeights, AMBuckets, Neurons, \
    TATTapPoint, TATAccumulator
from neuromorph.netobjnode import NetObjNode 


class Test_netobjnode(unittest.TestCase):
    def test_create_resources_for_Input(self):
        net_obj_dims = 3
       
        inp = ps.Input("Input", net_obj_dims)
        n = NetObjNode(inp)

        self.assertIsInstance(n.get_resource(), Source)
        self.assertEqual(n.get_net_obj(), inp)
        self.assertEqual(n.get_resource().D, net_obj_dims)

    def test_create_resources_for_Output(self):
        net_obj_dims = 3
        
        out = ps.Output("Output", net_obj_dims)
        n = NetObjNode(out)

        self.assertIsInstance(n.get_resource(), Sink)
        self.assertEqual(n.get_net_obj(), out)
        self.assertEqual(n.get_resource().D, net_obj_dims)

    def test_create_resources_for_Pool(self):
        net_obj_nrn_count = 30
        net_obj_dims = 3
        
        pool = ps.Pool("Pool", net_obj_nrn_count, net_obj_dims)
        n = NetObjNode(pool)

        self.assertIsInstance(n.get_resource(), Neurons)
        self.assertEqual(n.get_net_obj(), pool)
        self.assertEqual(n.get_resource().N, net_obj_nrn_count)

    def test_create_resources_for_Bucket(self):
        net_obj_dims = 3
        
        bkt = ps.Bucket("Bucket", net_obj_dims)
        n = NetObjNode(bkt)

        self.assertIsInstance(n.get_resource(), AMBuckets)
        self.assertEqual(n.get_net_obj(), bkt)
        self.assertEqual(n.get_resource().D, net_obj_dims)

    def test_create_resources_for_Weights(self):
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

        self.assertRaises(TypeError, NetObjNode.__init__, ps.Network("Network"))

    def test_add_adjacent_node(self):
        net_obj_dims = 3
        
        bkt_node = NetObjNode(ps.Bucket("Bucket", net_obj_dims))
        bkt2_node = NetObjNode(ps.Bucket("Bucket", net_obj_dims))

        net_obj_nrn_count = 30
        net_obj_dims = 3
        
        p1_node = NetObjNode(ps.Pool("Pool", net_obj_nrn_count, net_obj_dims))
        p1_node.add_adjacent_node(bkt_node)

        # one element in adjacent_nodes with first element having weights
        # equal to None and NetObjNode equal to bkt_node
        self.assertEqual(len(p1_node.adjacent_net_obj),1)
        self.assertEqual(p1_node.adjacent_net_obj[0][0],None)
        self.assertEqual(p1_node.adjacent_net_obj[0][1],bkt_node)

        w = ps.Weights(np.zeros((3,3)))
        p1_node.add_adjacent_node(bkt2_node, w)

        # two elements in adjacent_nodes with second element having weights
        # equal to Weights and NetObjNode equal to bkt_node
        self.assertEqual(len(p1_node.adjacent_net_obj), 2)
        self.assertEqual(p1_node.adjacent_net_obj[1][0], w)
        self.assertEqual(p1_node.adjacent_net_obj[1][1], bkt2_node)


    def test_connect__no_adj_nodes(self):
        net_obj_dims = 3
        resources = []

        bkt_node = NetObjNode(ps.Bucket("Bucket", net_obj_dims))
        bkt_node.connect(resources)

        self.assertEqual(len(resources), 0)

    def test_connect__one_adj_node__Input_Pool(self):
        net_obj_dims = 2
       
        inp_node = NetObjNode(ps.Input("Input", net_obj_dims))
        p1_node = NetObjNode(ps.Pool("Pool", 30, net_obj_dims))

        resources = []

        resources.append(inp_node.get_resource())
        resources.append(p1_node.get_resource())

        inp_node.add_adjacent_node(p1_node)
        inp_node.connect(resources)

        self.assertEqual(len(resources),3)
        self.assertEqual(inp_node.get_resource(),resources[0])
        self.assertEqual(p1_node.get_resource(),resources[1])
        self.assertIsInstance(resources[2],TATTapPoint)
        self.assertEqual(resources[2].DI(), 
                         inp_node.get_net_obj().get_num_dimensions())
        self.assertEqual(resources[2].DO(), 
                         p1_node.get_net_obj().get_num_neurons())
        self.assertEqual(resources[2].K, 
                         inp_node.get_net_obj().get_num_dimensions())

    def test_connect__one_adj_node__Input_Bucket__no_weights(self):
        net_obj_dims = 2
       
        inp_node = NetObjNode(ps.Input("Input", net_obj_dims))
        bkt_node = NetObjNode(ps.Bucket("Bucket", net_obj_dims))

        resources = []

        resources.append(inp_node.get_resource())
        resources.append(bkt_node.get_resource())

        inp_node.add_adjacent_node(bkt_node)
        self.assertRaises(TypeError, inp_node.connect, resources)

    def test_connect__one_adj_node__Input_Bucket(self):
        net_obj_dims = 2
       
        inp_node = NetObjNode(ps.Input("Input", net_obj_dims))
        bkt_node = NetObjNode(ps.Bucket("Bucket", net_obj_dims))

        resources = []

        resources.append(inp_node.get_resource())
        resources.append(bkt_node.get_resource())

        w = ps.Weights(np.zeros((net_obj_dims,net_obj_dims),dtype=np.uint32))
        inp_node.add_adjacent_node(bkt_node,w)
        inp_node.connect(resources)

        self.assertEqual(len(resources), 4)
        self.assertEqual(inp_node.get_resource(),resources[0])
        self.assertEqual(bkt_node.get_resource(),resources[1])
        self.assertIsInstance(resources[2],TATAccumulator)
        self.assertIsInstance(resources[3],MMWeights)
        self.assertEqual(resources[2].DI(), 
                         inp_node.get_net_obj().get_num_dimensions())
        self.assertEqual(resources[2].DO(), 
                         bkt_node.get_net_obj().get_num_dimensions())

    def test_connect__one_adj_node__Pool_Bucket(self):
        pass

    def test_connect__one_adj_node__Bucket_Bucket(self):
        pass

    def test_connect__one_adj_node__Bucket_Output(self):
        pass

    def test_connect__mult_adj_nodes__Bucket_Bucket(self):
        pass

    def test_connect__mult_adj_nodes__Bucket_Output(self):
        pass
