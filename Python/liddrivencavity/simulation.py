from pyfluids import Simulation
from pyfluids.boundaryconditions import NoSlipBoundaryCondition, VelocityBoundaryCondition
from pyfluids.geometry import GbCuboid3D
from pyfluids.kernel import LBMKernel, KernelType
from pyfluids.parameters import GridParameters, PhysicalParameters, RuntimeParameters
from pyfluids.writer import Writer, OutputFormat
from pymuparser import Parser

runtime_params = RuntimeParameters()
runtime_params.number_of_threads = 4
runtime_params.number_of_timesteps = 10000
runtime_params.timestep_log_interval = 1000

physical_params = PhysicalParameters()
physical_params.lattice_viscosity = 0.005

grid_params = GridParameters()
grid_params.number_of_nodes_per_direction = [64, 64, 64]
grid_params.blocks_per_direction = [2, 2, 2]
grid_params.node_distance = 1 / 10


def run_simulation(physical_params=physical_params, grid_params=grid_params, runtime_params=runtime_params):
    simulation = Simulation()
    kernel = LBMKernel(KernelType.CompressibleCumulantFourthOrderViscosity)

    writer = Writer()
    writer.output_path = "./output"
    writer.output_format = OutputFormat.BINARY

    simulation.set_grid_parameters(grid_params)
    simulation.set_physical_parameters(physical_params)
    simulation.set_simulation_parameters(runtime_params)
    simulation.set_kernel_config(kernel)
    simulation.set_writer(writer)

    no_slip_bc_adapter = NoSlipBoundaryCondition()

    fct = Parser()
    fct.expression = "u"
    fct.define_constant("u", 0.005)
    velocity_bc_adapter = VelocityBoundaryCondition(True, True, False, fct, 0, -10.0)

    g_min_x1, g_min_x2, g_min_x3 = 0, 0, 0
    g_max_x1 = grid_params.number_of_nodes_per_direction[0] * grid_params.node_distance
    g_max_x2 = grid_params.number_of_nodes_per_direction[1] * grid_params.node_distance
    g_max_x3 = grid_params.number_of_nodes_per_direction[2] * grid_params.node_distance

    dx = grid_params.node_distance

    wall_x_min = GbCuboid3D(g_min_x1 - dx, g_min_x2 - dx, g_min_x3 - dx, g_min_x1, g_max_x2 + dx, g_max_x3)
    wall_x_max = GbCuboid3D(g_max_x1, g_min_x2 - dx, g_min_x3 - dx, g_max_x1 + dx, g_max_x2 + dx, g_max_x3)
    wall_y_min = GbCuboid3D(g_min_x1 - dx, g_min_x2 - dx, g_min_x3 - dx, g_max_x1 + dx, g_min_x2, g_max_x3)
    wall_y_max = GbCuboid3D(g_min_x1 - dx, g_max_x2, g_min_x3 - dx, g_max_x1 + dx, g_max_x2 + dx, g_max_x3)
    wall_z_min = GbCuboid3D(g_min_x1 - dx, g_min_x2 - dx, g_min_x3 - dx, g_max_x1 + dx, g_max_x2 + dx, g_min_x3)
    wall_z_max = GbCuboid3D(g_min_x1 - dx, g_min_x2 - dx, g_max_x3, g_max_x1 + dx, g_max_x2 + dx, g_max_x3 + dx)

    simulation.add_object(wall_x_min, no_slip_bc_adapter, 1, "/geo/wallXmin")
    simulation.add_object(wall_x_max, no_slip_bc_adapter, 1, "/geo/wallXmax")
    simulation.add_object(wall_y_min, no_slip_bc_adapter, 1, "/geo/wallYmin")
    simulation.add_object(wall_y_max, no_slip_bc_adapter, 1, "/geo/wallYmax")
    simulation.add_object(wall_z_min, no_slip_bc_adapter, 1, "/geo/wallZmin")
    simulation.add_object(wall_z_max, velocity_bc_adapter, 1, "/geo/wallZmax")

    simulation.run_simulation()


if __name__ == "__main__":
    run_simulation()
