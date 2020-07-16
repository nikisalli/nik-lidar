import numpy as np
import open3d.open3d as o3d

if __name__ == "__main__":
    pcd = o3d.io.read_point_cloud("/home/nik/kek.xyz", format='xyz')
    vis = o3d.visualization.Visualizer()
    vis.create_window()
    vis.add_geometry(pcd)
    opt = vis.get_render_option()
    opt.background_color = np.asarray([0.15, 0.15, 0.15])
    vis.run()
    vis.destroy_window()