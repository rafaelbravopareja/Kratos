from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# Other imports
from python_solver import PythonSolver


def CreateSolver(model, custom_settings):
    '''This function creates the requested solver
    If no "ale_settings" are specified a regular fluid-solver is created
    '''
    if custom_settings["solver_settings"].Has("ale_settings"):
        return ALEFluidSolver(model, custom_settings)
    else:
        KratosMultiphysics.Logger.PrintInfo("ALEFluidSolver", "No ale settings found, creating a pure fluid solver")
        KratosMultiphysics.CheckRegisteredApplications("FluidDynamicsApplication")
        import python_solvers_wrapper_fluid
        return python_solvers_wrapper_fluid.CreateSolver(model, custom_settings)


class ALEFluidSolver(PythonSolver):
    def __init__(self, model, custom_settings):
        super(ALEFluidSolver, self).__init__(model, custom_settings["solver_settings"])

        fluid_model_part_name = custom_settings["solver_settings"]["model_part_name"].GetString()
        if not self.model.HasModelPart(fluid_model_part_name):
            fluid_mesh_model_part = KratosMultiphysics.ModelPart(fluid_model_part_name)
            self.model.AddModelPart(fluid_mesh_model_part)

        ## Creating the fluid solver
        self.fluid_solver = self._CreateFluidSolver(custom_settings)

        ## Creating the mesh-motion solver
        KratosMultiphysics.CheckRegisteredApplications("MeshMovingApplication")
        mesh_motion_settings = custom_settings.Clone()
        # delete the (fluid) solver settings
        mesh_motion_settings.RemoveValue("solver_settings")
        # add the ale solver settings as solver settings
        mesh_motion_settings.AddValue("solver_settings", custom_settings["solver_settings"]["ale_settings"])
        mesh_motion_solver_settings = mesh_motion_settings["solver_settings"]

        if not mesh_motion_solver_settings.Has("echo_level"):
            mesh_motion_solver_settings.AddEmptyValue("echo_level")
            fluid_echo_lvl = custom_settings["solver_settings"]["echo_level"].GetInt()
            mesh_motion_solver_settings["echo_level"].SetInt(fluid_echo_lvl)

        if mesh_motion_solver_settings.Has("model_part_name"):
            if not fluid_model_part_name == mesh_motion_solver_settings["model_part_name"].GetString():
                raise Exception('Fluid- and Mesh-Solver have to use the same "model_part_name"!')
        else:
            mesh_motion_solver_settings.AddEmptyValue("model_part_name")
            fluid_model_part_name = custom_settings["solver_settings"]["model_part_name"].GetString()
            mesh_motion_solver_settings["model_part_name"].SetString(fluid_model_part_name)

        if mesh_motion_solver_settings.Has("domain_size"):
            fluid_domain_size = custom_settings["solver_settings"]["domain_size"].GetInt()
            mesh_motion_domain_size = mesh_motion_solver_settings["domain_size"].GetInt()
            if not fluid_domain_size == mesh_motion_domain_size:
                raise Exception('Fluid- and Mesh-Solver have to use the same "domain_size"!')
        else:
            mesh_motion_solver_settings.AddEmptyValue("domain_size")
            fluid_domain_size = custom_settings["solver_settings"]["domain_size"].GetInt()
            mesh_motion_solver_settings["domain_size"].SetInt(fluid_domain_size)

        # Get the names of the interface-parts for which the MESH_VELOCITY should be
        # applied to the Fluid-VELOCITY
        self.ale_interface_part_names = []
        if mesh_motion_solver_settings.Has("ale_interface_parts"):
            for i in range(mesh_motion_solver_settings["ale_interface_parts"].size()):
                self.ale_interface_part_names.append(
                    mesh_motion_solver_settings["ale_interface_parts"][i].GetString())
            mesh_motion_solver_settings.RemoveValue("ale_interface_parts")

        import python_solvers_wrapper_mesh_motion
        self.mesh_motion_solver = python_solvers_wrapper_mesh_motion.CreateSolver(model, mesh_motion_settings)

        # Getting the min_buffer_size from both solvers
        # and assigning it to the fluid_solver, bcs this one handles the model_part
        # self.fluid_solver.min_buffer_size = max(self.fluid_solver.GetMinimumBufferSize(),
        #                                         self.mesh_motion_solver.GetMinimumBufferSize())

        self.is_printing_rank = self.fluid_solver._IsPrintingRank()

        # TODO move to "Check"?
        if (self.mesh_motion_solver.settings["calculate_mesh_velocities"].GetBool() == False
            and self.is_printing_rank):
            info_msg = "Mesh velocities are not being computed in the Mesh solver!"
            KratosMultiphysics.Logger.PrintInfo("::[ALEFluidSolver]::", info_msg)

        if (self.fluid_solver.settings["compute_reactions"].GetBool() == False
            and self.is_printing_rank):
            info_msg = "Reactions are not being computed in the Fluid solver!"
            KratosMultiphysics.Logger.PrintInfo("::[ALEFluidSolver]::", info_msg)

        if self.is_printing_rank:
            KratosMultiphysics.Logger.PrintInfo("::[ALEFluidSolver]::", "Construction finished")

    def AddVariables(self):
        self.mesh_motion_solver.AddVariables()
        self.fluid_solver.AddVariables()
        if self.is_printing_rank:
            KratosMultiphysics.Logger.PrintInfo("::[ALEFluidSolver]::", "Variables Added")

    def AddDofs(self):
        self.mesh_motion_solver.AddDofs()
        self.fluid_solver.AddDofs()
        if self.is_printing_rank:
            KratosMultiphysics.Logger.PrintInfo("::[ALEFluidSolver]::", "DOFs Added")

    def Initialize(self):
        self.mesh_motion_solver.Initialize()
        self.fluid_solver.Initialize()
        if self.is_printing_rank:
            KratosMultiphysics.Logger.PrintInfo("::[ALEFluidSolver]::", "Finished initialization")

    def ImportModelPart(self):
        self.fluid_solver.ImportModelPart() # only ONE solver imports the ModelPart

    def PrepareModelPart(self):
        # Doing it ONLY for the fluid solver (since this contains filling the buffer)
        return self.fluid_solver.PrepareModelPart()

    def AdvanceInTime(self, current_time):
        # Doing it ONLY for the fluid solver
        return self.fluid_solver.AdvanceInTime(current_time)

    def Finalize(self):
        self.mesh_motion_solver.Finalize()
        self.fluid_solver.Finalize()

    def InitializeSolutionStep(self):
        self.mesh_motion_solver.InitializeSolutionStep()
        self.fluid_solver.InitializeSolutionStep()

    def Predict(self):
        self.mesh_motion_solver.Predict()
        self.fluid_solver.Predict()

    def FinalizeSolutionStep(self):
        self.mesh_motion_solver.FinalizeSolutionStep()
        self.fluid_solver.FinalizeSolutionStep()

    def SolveSolutionStep(self):
        self.mesh_motion_solver.SolveSolutionStep()

        # Copy the MESH_VELOCITY to the VELOCITY (ALE) on the interface
        for smp_name in self.ale_interface_part_names:
            part_nodes = self.GetComputingModelPart().GetSubModelPart(smp_name).Nodes
            KratosMultiphysics.VariableUtils().CopyVectorVar(KratosMultiphysics.MESH_VELOCITY,
                                                             KratosMultiphysics.VELOCITY,
                                                             part_nodes)

        self.fluid_solver.SolveSolutionStep()

    def Check(self):
        self.mesh_motion_solver.Check()
        self.fluid_solver.Check()

    def Clear(self):
        self.mesh_motion_solver.Clear()
        self.fluid_solver.Clear()

    def GetComputingModelPart(self):
        return self.mesh_motion_solver.GetComputingModelPart() # this is the same as the one used in Fluid

    def GetFluidSolver(self):
        return self.fluid_solver

    def GetMeshMotionSolver(self):
        return self.mesh_motion_solver

    def MoveMesh(self):
        self.GetMeshMotionSolver().MoveMesh()


    def _CreateFluidSolver(self, custom_settings):
        '''This function creates the fluid solver.
        It can be overridden to create different fluid solvers
        '''
        KratosMultiphysics.CheckRegisteredApplications("FluidDynamicsApplication")
        import python_solvers_wrapper_fluid
        fluid_settings = custom_settings.Clone()
        # remove the ale_settings so we can use the fluid_solver_wrapper constructor
        fluid_settings["solver_settings"].RemoveValue("ale_settings")
        return python_solvers_wrapper_fluid.CreateSolver(self.model, fluid_settings)