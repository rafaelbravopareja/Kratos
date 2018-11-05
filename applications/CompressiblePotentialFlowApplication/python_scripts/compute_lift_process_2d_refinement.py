from KratosMultiphysics import *
import KratosMultiphysics
from numpy import *
import itertools
import loads_output
import math

def Factory(settings, Model):
    if( not isinstance(settings,KratosMultiphysics.Parameters) ):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return ComputeLiftProcess(Model, settings["Parameters"])

##all the processes python processes should be derived from "python_process"
class ComputeLiftProcess(KratosMultiphysics.Process):
    def __init__(self, Model, settings ):
        KratosMultiphysics.Process.__init__(self)

        default_parameters = KratosMultiphysics.Parameters("""
            {
                "model_part_name":"PLEASE_CHOOSE_MODEL_PART_NAME",
                "upper_surface_model_part_name" : "please specify the model part that contains the upper surface nodes",
                "lower_surface_model_part_name" : "please specify the model part that contains the lower surface nodes",
                "far_field_model_part_name"   : "PotentialWallCondition2D_Far_field_Auto1",
                "mesh_id": 0,
                "velocity_infinity": [1.0,0.0,0],
                "angle_of_attack": 0.0,
                "reference_area": 1
            }  """)

        settings.ValidateAndAssignDefaults(default_parameters)

        self.fluid_model_part = Model[settings["model_part_name"].GetString()]
        self.upper_surface_model_part = Model[settings["upper_surface_model_part_name"].GetString()]
        self.lower_surface_model_part = Model[settings["lower_surface_model_part_name"].GetString()]
        self.far_field_model_part = Model[settings["far_field_model_part_name"].GetString()]

        self.velocity_infinity = [0,0,0]
        self.velocity_infinity[0] = settings["velocity_infinity"][0].GetDouble()
        self.velocity_infinity[1] = settings["velocity_infinity"][1].GetDouble()
        self.velocity_infinity[2] = settings["velocity_infinity"][2].GetDouble()

        self.reference_area =  settings["reference_area"].GetDouble()
        self.aoa = settings["angle_of_attack"].GetDouble()
        self.cl_reference = loads_output.read_cl_reference(self.aoa)

    def ExecuteFinalizeSolutionStep(self):
        print('COMPUTE LIFT')

        rx = 0.0
        ry = 0.0
        rz = 0.0
        
        rx_low = 0.0
        ry_low = 0.0
        rz_low = 0.0

        self.work_dir = '/home/inigo/simulations/naca0012/07_salome/05_MeshRefinement/'
        cp_results_file_name = self.work_dir + 'plots/cp/data/0_original/cp_results.dat'
        cp_file = open(cp_results_file_name,'w')

        #cp_coordinates_file_name = 'plots/cp/data/0_original/coordinates.dat'
        #coordinates_file = open(cp_coordinates_file_name,'a')
        number_of_conditions = 0
        for cond in itertools.chain(self.upper_surface_model_part.Conditions, self.lower_surface_model_part.Conditions):
            number_of_conditions += 1

        factor = math.floor(number_of_conditions / 7000+1)
        
        
        condition_counter = 0
        for cond in itertools.chain(self.upper_surface_model_part.Conditions, self.lower_surface_model_part.Conditions):
          condition_counter +=1
          n = cond.GetValue(NORMAL)
          cp_up = cond.GetValue(PRESSURE)
          cp_low = cond.GetValue(KratosMultiphysics.CompressiblePotentialFlowApplication.PRESSURE_LOWER)

          x = 0.5*(cond.GetNodes()[1].X0+cond.GetNodes()[0].X0)
          y = 0.5*(cond.GetNodes()[1].Y0+cond.GetNodes()[0].Y0)

          if(number_of_conditions > 7000):
              if( condition_counter % factor == 0 ):
                  cp_file.write('{0:15f} {1:15f}\n'.format(x+0.5, cp_up))
          else:
              cp_file.write('{0:15f} {1:15f}\n'.format(x+0.5, cp_up))
          
          #cp_file.write('{0:15f} {1:15f}\n'.format(x+0.5, cp_up))
          #coordinates_file.write('{0:15f} {1:15f}\n'.format(x+0.5, -y))
          


          rx += n[0]*cp_up
          ry += n[1]*cp_up
          rz += n[2]*cp_up
          
          rx_low += n[0]*cp_low
          ry_low += n[1]*cp_low
          rz_low += n[2]*cp_low
        
        cp_file.flush()

        print('number of conditions = ', number_of_conditions)
        
        RX = rx/self.reference_area
        RY = ry/self.reference_area
        RZ = rz/self.reference_area        
        
        RX_low = rx_low/self.reference_area
        RY_low = ry_low/self.reference_area
        RZ_low = rz_low/self.reference_area

        Cl = RY
        Cd = RX

        self.Cl = Cl

        relative_error = abs(Cl - self.cl_reference)/abs(self.cl_reference)*100.0
        
        Cl_low = RY_low
        Cd_low = RX_low

        print('Cl = ', Cl)
        print('Cd = ', Cd)
        print('RZ = ', RZ)        

        print('\nCl_low = ', Cl_low)
        print('Cd_low = ', Cd_low)
        print('RZ_low = ', RZ_low)
        
        print("\nLift_Difference = ", Cl - Cl_low)
        #print('Mach = ', self.velocity_infinity[0]/340)
        
        
        for node in self.far_field_model_part.Nodes:
            jump = node.GetSolutionStepValue(KratosMultiphysics.CompressiblePotentialFlowApplication.POTENTIAL_JUMP)
            if(abs(jump - 1e-8) > 1e-7):
                far_field_lift = jump
                print('Far field computed lift = ', far_field_lift)
                break

        relative_error_jump = abs(far_field_lift - self.cl_reference)/abs(self.cl_reference)*100.0
        NumberOfNodes = self.fluid_model_part.NumberOfNodes()
    
        with open (self.work_dir + "mesh_refinement_loads.dat",'a') as loads_file:
            loads_file.write('{0:16.2e} {1:15f} {2:15f} {3:15f} {4:15f} {5:15f} {6:15f}\n'.format(NumberOfNodes, Cl, Cl_low, far_field_lift, self.cl_reference, Cd, Cd_low, RZ))
            loads_file.flush()

        with open(self.work_dir + "plots/results/all_cases.dat",'a') as aoa_file:
            aoa_file.write('{0:16.2e} {1:15f} {2:15f} {3:15f} {4:15f} {5:15f} {6:15f}\n'.format(NumberOfNodes, Cl, Cl_low, far_field_lift, self.cl_reference, Cd, Cd_low, RZ))
            aoa_file.flush()

        cl_results_file_name = self.work_dir + "plots/cl/data/cl/cl_results.dat"
        with open(cl_results_file_name,'a') as cl_file:
            cl_file.write('{0:16.2e} {1:15f}\n'.format(NumberOfNodes, Cl))
            cl_file.flush()

        cl_error_results_file_name = self.work_dir + "plots/cl_error/data/cl/cl_error_results.dat"
        with open(cl_error_results_file_name,'a') as cl_error_file:
            cl_error_file.write('{0:16.2e} {1:15f}\n'.format(NumberOfNodes, relative_error))
            cl_error_file.flush()

        cl_far_field_results_file_name = self.work_dir + "plots/cl/data/cl/cl_jump_results.dat"
        with open(cl_far_field_results_file_name,'a') as cl_jump_file:
            cl_jump_file.write('{0:16.2e} {1:15f}\n'.format(NumberOfNodes, far_field_lift))
            cl_jump_file.flush()

        cl_far_field_error_results_file_name = self.work_dir + "plots/cl_error/data/cl/cl_jump_error_results.dat"
        with open(cl_far_field_error_results_file_name,'a') as cl_jump_error_file:
            cl_jump_error_file.write('{0:16.2e} {1:15f}\n'.format(NumberOfNodes, relative_error_jump))
            cl_jump_error_file.flush()
        
        cd_results_file_name = self.work_dir + "plots/cd/data/cd/cd_results.dat"
        with open(cd_results_file_name,'a') as cd_file:
            cd_file.write('{0:16.2e} {1:15f}\n'.format(NumberOfNodes, Cd))
            cd_file.flush()

        cp_tikz_file_name = self.work_dir + "plots/cp/data/0_original/cp.tikz"
        with open(cp_tikz_file_name,'w') as cp_tikz_file:
            cp_tikz_file.write('\\begin{tikzpicture}\n' +
            '\\begin{axis}[\n' +
            '\t    title={ $c_l$ = ' + "{:.6f}".format(Cl) + ' $c_d$ = ' + "{:.6f}".format(Cd) + '},\n' +
            '\t    xlabel={$x/c$},\n' +
            '\t    ylabel={$c_p[\\unit{-}$]},\n' +
            '\t    xmin=-0.01, xmax=1.01,\n' +
            '\t    y dir=reverse,\n' +
            '\t    xtick={0,0.2,0.4,0.6,0.8,1},\n' +
            '\t    xticklabels={0,0.2,0.4,0.6,0.8,1},\n' +
            '\t    ymajorgrids=true,\n' +
            '\t    xmajorgrids=true,\n' +
            '\t    grid style=dashed,\n' +
            '\t    width=12cm\n' +
            ']\n\n' +
            '\\addplot[\n' +
            '\t    only marks,\n' +
            '\t    color=blue,\n' +
            '\t    mark=*,\n' +
            '\t    ]\n' +
            '\t    table {cp_results.dat};  \n' +
            '\t    \\addlegendentry{ndof = ' + "{:.2e}".format(NumberOfNodes) + ' }\n\n' +
            '\t\end{axis}\n' +
            '\t\end{tikzpicture}')
            cp_tikz_file.flush()

        #loads_output.write_cl(Cl)

    def ExecuteFinalize(self):
        with open(self.work_dir + 'plots/aoa/cl_aoa.dat','a') as cl_aoa_file:
            cl_aoa_file.write('{0:15f}\n'.format(self.Cl))
            cl_aoa_file.flush()