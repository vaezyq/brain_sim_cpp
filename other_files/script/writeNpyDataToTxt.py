import numpy as np
import unittest


class WriteNpyDataToTxt(unittest.TestCase):
    # 将map表写入txt文件
    @staticmethod
    def _test_write_map_npy_to_txt():
        map_file_path = "../tables/map_tables/map_2000/map_2000_after_size_balance_without_invalid_index.npy"
        txt_file_write_path = "../tables/map_tables/map_2000/map_2000_after_size_balance.txt"
        map_table = np.load(map_file_path, allow_pickle=True)
        with open(txt_file_write_path, 'wb') as f:
            for i in range(len(map_table)):
                key_value = []
                for k, v in map_table[i].items():
                    key_value.append(int(k))
                    key_value.append(float(v))
                # print(key_value)
                # f.write( ''.join(key_value))
                str = " ".join('%s' % id for id in key_value)
                # print(str)
                str = str.encode(encoding='utf-8')
                f.write(str)
                f.write("\n".encode('utf-8'))
        # np.savetxt(f, degree)
        f.close()

    # 将流量数组写入txt文件

    @staticmethod
    def test_write_traffic_npy_to_txt():
        traffic_file_path = "../tables/traffic_tables/traffic_2000/2_dim/traffic_table_base_dcu_out_in_2_dimmap_2000_sequential_cortical_v2.npy"
        txt_file_write_path = "../tables/traffic_tables/traffic_2000/2_dim/traffic_table_out_in_2_dim_map_2000_balance_size_map.txt"
        map_table = np.load(traffic_file_path, allow_pickle=True)
        with open(txt_file_write_path, 'wb') as f:
            for i in range(len(map_table)):
                str = " ".join('%d' % id for id in map_table[i])
                # print(str)
                str = str.encode(encoding='utf-8')
                f.write(str)
                f.write("\n".encode('utf-8'))
        # np.savetxt(f, degree)
        f.close()


if __name__ == '__main__':
    unittest.main()
