/*
//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application
//
//  Main authors:   Lukas Rauch
*/

// System includes
#include "includes/define.h"
#include "includes/variables.h"
#include "utilities/math_utils.h"
#include <typeinfo>

// #include <math.h>
// #include "Tools.h"
#include <fstream>

// External includes

// Project includes
#include "iga_base_element.h"
#include "iga_beam_element.h"
#include "iga_application_variables.h"
#include "custom_elements/base_discrete_element.h"
#include "custom_utilities/iga_debug.h"

#define LOG(x) std::cout << x << std::endl;

namespace Kratos {

Element::Pointer IgaBeamElement::Create(
    IndexType NewId,
    NodesArrayType const& ThisNodes,
    PropertiesType::Pointer pProperties) const
{
    auto geometry = GetGeometry().Create(ThisNodes);

    return Kratos::make_shared<IgaBeamElement>(NewId, geometry,
        pProperties);
}

void IgaBeamElement::GetDofList(
    DofsVectorType& rElementalDofList,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    rElementalDofList.resize(NumberOfDofs());

    for (std::size_t i = 0; i < NumberOfNodes(); i++) {
        SetElementDof(rElementalDofList, i, 0, DISPLACEMENT_X);
        SetElementDof(rElementalDofList, i, 1, DISPLACEMENT_Y);
        SetElementDof(rElementalDofList, i, 2, DISPLACEMENT_Z);
        SetElementDof(rElementalDofList, i, 3, DISPLACEMENT_ROTATION);
    }

    KRATOS_CATCH("")
}

void IgaBeamElement::EquationIdVector(
    EquationIdVectorType& rResult,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    rResult.resize(NumberOfDofs());

    for (std::size_t i = 0; i < NumberOfNodes(); i++) {
        SetElementEquationId(rResult, i, 0, DISPLACEMENT_X);
        SetElementEquationId(rResult, i, 1, DISPLACEMENT_Y);
        SetElementEquationId(rResult, i, 2, DISPLACEMENT_Z);
        SetElementEquationId(rResult, i, 3, DISPLACEMENT_ROTATION);
    }

    KRATOS_CATCH("")
}

void IgaBeamElement::Initialize()
{
}

void IgaBeamElement::CalculateAll(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo,
    const bool ComputeLeftHandSide,
    const bool ComputeRightHandSide)
{
    KRATOS_TRY;

    // tmporary debug data
    auto expected_data = Parameters(GetValue(DEBUG_EXPECTED_DATA));
   

    // get integration data
    
    Vector3 t0 = GetValue(T0);
    const double integration_weight = GetValue(INTEGRATION_WEIGHT);
    Vector& shape_function_values = GetValue(SHAPE_FUNCTION_VALUES);
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
    /// Testprit
    // IgaDebug::CheckVector(expected_data,"shape_0", shape_function_values);

    // get properties
    const auto& properties = GetProperties();

    const double _emod = properties[YOUNG_MODULUS];
    const double _gmod = properties[SHEAR_MODULUS];
    const double _area = properties[CROSS_AREA];
    const double prestress = properties[PRESTRESS_CAUCHY];
    const double Phi = properties[PHI]; 
    const double Phi_der = properties[PHI_0_DER];
    const double _m_inert_y = properties[MOMENT_OF_INERTIA_Y];
    const double _m_inert_z = properties[MOMENT_OF_INERTIA_Z];
    const double _mt_inert = properties[MOMENT_OF_INERTIA_T];

 
    // TODO: Add stiffness andd force



    // Reference and Actual Configuration
    Vector3 R1;
    Vector3 R2;
    double A;
    double B;
    Vector3 r1;
    Vector3 r2;
    double a;
    double b;

    ComputeGeometryReference(R1, R2, A, B);
    ComputeGeometryActual(r1, r2, a, b);

    // Create empty Stiffness Matrix
    MatrixType _gke; 
    VectorType _gfie; 

    ElementStiffnessMatrixNonlinear(_emod, _gmod, _area, _m_inert_y, _m_inert_z, _mt_inert, integration_weight, _gke, _gfie);
    // IgaDebug::CheckVector(expected_data,"stiffness", _gke);
    // IgaDebug::CheckVector(expected_data,"external_force", _gkfi);



    
    KRATOS_CATCH("");
}


// //#################################################################################
// //#################################################################################
// //#
// //#                         +++ Iga_Beam_Element +++
// //#
// //#################################################################################
// //#################################################################################
// //#
// /** 
//      * 
//      * @return
//      * 
//      * @param[out]   
//      * 
//      * @author L.Rauch (10/2018)
//      * 
//      * @note by A.Bauer (10/2014)
//      */
// //#--------------------------------------------------------------------------------
// void IgaBeamElement::IgaBeamElement(
//         int _ID, 
//         Part* _Part_Prt,
//         std::vector<Node*> _Node_Vec,
//         MaterialBasis* _Mat_Prt,
//         PropertyIgaBeamElement* _Prop_Prt,
//         Nurbs1D* _Nurbs_Curve,
//         int _knotspan_index, 
//         std::vector<std::vector<double,2>> _phi_axis_n,
//         std::vector<double,3> _t0,
//         std::vector<EvalPtBasis*> _eval_points, 
//         KnotSpan_Belonging _elementbelongig, 
//         PropertyNURBS_Curve_Stab* _prop_stab )
//     :ElementCurve_Nurbs(
//         _ID, _Part_Prt, _Node_Vec, _Mat_Prt, 
//         _Nurbs_Curve, _knotspan_index, _Prop_Prt,
//         _eval_points, _elementbelongig, _prop_stab),
//         Prop_Prt(_Prop_Prt),
//         phi_axis_n(_phi_axis_n)

// {
// // Set disretisation type
// discretisation_type = nurbs1d; 

// //# TODO ### function [ GetPDeg() ] implementieren
// p_deg = _Nurbs_Curve -> GetPDeg();

// dof_node = 4

// N_dof = dof_node * (p_deg+1); 

// // Activate the DOSs at each Node that needed for this Element Type (ElementMembrane1)
// std::vector<int> act_dofs; 

// // get Actual DOFs
// GetDofTypesPerNode(act_dofs);

// //# TODO ### funktion [Set_Active_DOFs() ] implementieren
// for(size_t i=0; i < Node_Vec.size(); i++ ){
//     Node_Vec[i] -> SetActiveDOFs(act_dofs);
// }

// // Set Tollereance Level

// int tol = 1.0e-8;

// }  




// #################################################################################
// #################################################################################
// #
// #                       +++ Get_Dof_Types_PerNode +++
// #
// #################################################################################
// #################################################################################
// #
/** gets the degree of freedom types of this element at each node
     * 
     * @return
     * 
     * @param[out]   _act_dofs       Acting Degrees of Freedom per Node
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (10/2014)
     */
// #--------------------------------------------------------------------------------

void IgaBeamElement::GetDofTypesPerNode(
    std::vector<int>& _act_dofs){
    
//# TODO ### die Anzahl der Freiheitsgrade pro Node festlegen. Irgendwo ganz am Anfang ( in caratt++ in erster Funtion)
    int dof_node = 4;

    _act_dofs.resize(dof_node);         // _act_dof Vektor auf richtige Länge einstellen
    _act_dofs[0] = DISPLACEMENT_X;
    _act_dofs[1] = DISPLACEMENT_Y;
    _act_dofs[2] = DISPLACEMENT_Z; 
    if(dof_node == 4 ) _act_dofs[3] = DISPLACEMENT_ROTATION;
}

// #################################################################################
// #################################################################################
// #
// #                          +++ Iga_Beam_Element +++
// #
// #################################################################################
// #################################################################################
// #
/** 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (10/2014)
     */
// #--------------------------------------------------------------------------------

IgaBeamElement::~IgaBeamElement(void)
{ 
}



//#################################################################################
//#################################################################################
//#
//#                 +++ Stiffnes Matrix Element Linear +++
//#
//#################################################################################
//#################################################################################
//#
/** Evaluates the Stiffnes Matrix and Residual Vector at the Integration Point
     * 
     * @param[in]   _u_act        NURBS coordinates of integration pont u-direction
     * @param[in]   _emod         Youns Modulus
     * @param[in]   _area         Area of Cross Section
     * @param[in]   _m_inert      Moment of Intertia
     * @param[out]  _gke          Elemental Stiffness Matrix per Gauss Point
     * 
     * @author  L.Rauch (10/2018)
     * 
     * @note    By M.Breitenberger(08/2012)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::StffnessMatrixElementLinear(
    double _emod,
    double _gmod,
    double _area,
    double _m_inert_n,
    double _m_inert_v,
    double _mt_inert,
    double _dl,
        MatrixType& gke)
{
    gke.resize(NumberOfDofs(), NumberOfDofs());

    Vector& shape_function = GetValue(SHAPE_FUNCTION_VALUES);
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    // Declaration 
    Vector3 R_1;   // 1st Derivative of the Curve Undeformed Config
    Vector3 R_2;   // 2nd Derivative of the Curve Undeformed Config
    double A;               // Length of the Base Vector
    double B;               // Curvature Including the Metric
    double B_n;
    double B_v;
    double C_12;
    double C_13;
    double Phi;
    double Phi_der;

    Vector3 r_1;   // 1st Derivative of the Curve Deformed Config. 
    Vector3 r_2;   // 2nd DErivative of the Curve Deformed Config.
    double a;               // Length of the Base Vector in Deformed Config.
    double b;               // Curvatur Including the Metric in Defomred Config. 
    double b_n;
    double b_v;
    double c_12;
    double c_13;
    double phi;
    double phi_der;
    
    // Declaration of Pricipal Axis
    Vector3 N;     // Principal Axis 1 of Cross Section in UNdeformed Config.
    Vector3 V;   // Prinzipal Axis 2 of Cross Section in UNdeformed Config. 
    Vector3 n;   // Principal Axis 1 Of Cross Section in Defomred Config.
    Vector3 v;  // Principal Axis 2 of Cross Section in Deformed Config. 
    Vector3 N0;    // Principal Axis 1 of Cross Section in Reference Config. 
    Vector3 V0;    // Principal Axis 2 of Cross Section in Reference Config. 


    // Material and Cross Section
    double emod_A = _emod * _area;
    double emod_I_n = _emod * _m_inert_n;
    double emod_I_v = _emod * _m_inert_v;
    double emod_It = _gmod * _mt_inert; 

    // Membrane Stiffness
    Matrix kem(NumberOfDofs(), NumberOfDofs());

    // Bending Stiffness (Principal Axis 1)
    Matrix keb_n(NumberOfDofs(), NumberOfDofs());

    // Bending Stiffness (Principal Axis 2) 
    Matrix keb_v(NumberOfDofs(), NumberOfDofs());

    // Torsional Stiffness
    Matrix ket_n(NumberOfDofs(), NumberOfDofs());
    Matrix ket_v(NumberOfDofs(), NumberOfDofs());


    // Get Previous Results
    Vector3 tmp_ini_disp;
    tmp_ini_disp.resize(4);
    tmp_ini_disp.clear();

    Phi = 0;
    Phi_der = 0;
    phi = 0;
    phi_der = 0;

    std::vector<int> act_dofs;
    GetDofTypesPerNode(act_dofs); 

    for (size_t i = 0; i < NumberOfNodes(); i++)
    {
        tmp_ini_disp = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT); 
        if (size(act_dofs) > 3)
        {
            phi += shape_function(i) * tmp_ini_disp[3];
            phi_der += shape_derivatives(0,i) * tmp_ini_disp[3];
        }
    }

    // Compute Geometry Reference
    ComputeGeometryReference(R_1, R_2, A, B);
    ComputeGeometryInitial(r_1, r_2, a, b);

    // ComputeCrossSectionGeometryReference(R_1, R_2, t0_0, N, V, N0, V0, B_n, B_v, C_12, C_13, Phi, Phi_der); 



}

//#################################################################################
//#################################################################################
//#
//#                 +++ Stiffness Matrix Element Nonlinear +++
//#
//#################################################################################
//#################################################################################
//#
/** Calculates the non-linear element stiffness matrix
     * 
     * @param[in]   _u_act        NURBS coordinates of integration pont u-direction
     * @param[in]   _emod         Youns Modulus
     * @param[in]   _area         Area of Cross Section
     * @param[in]   _m_inert      Moment of Intertia
     * @param[out]  _gke          Elemental Stiffness Matrix per Gauss Point
     * @param[out]  _gfie         Internal Force Vector per Gauss Point 
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ElementStiffnessMatrixNonlinear(
    double _emod,
    double _gmod,
    double _area,
    double _m_inert_y,
    double _m_inert_z,
    double _mt_inert,
    double _dL,
        MatrixType& _gke,
        VectorType& _gfie)
    {
    KRATOS_TRY;
        
    // tmporary debug data
    auto expected_data = Parameters(GetValue(DEBUG_EXPECTED_DATA));
    Vector3 T0_vec = GetValue(T0);

    // SetUp empty stiffness Matrix
    _gke.resize(NumberOfDofs(), NumberOfDofs());
    _gke.clear(); 

    // Vector _gfie;
    _gfie.resize(NumberOfDofs());
    _gfie.clear(); 

    // Material And Cross Section
    double emod_A = _emod * _area; 
    double emod_I_n = _emod * _m_inert_z;
    double emod_I_v = _emod * _m_inert_y; 
    double gmod_It = _gmod * _mt_inert; 

    // Declarate Empty Stiffness Matixes

    // Membrane Stiffness Matrix
    Matrix kem(NumberOfDofs(), NumberOfDofs());
    kem.clear();

    // Bendign Stiffness Matrix (Principal Axis N)
    Matrix keb_n(NumberOfDofs(), NumberOfDofs());
    keb_n.clear();

    // Bendign Stiffness Matrix (Principal Axis V)
    Matrix keb_v(NumberOfDofs(), NumberOfDofs());
    keb_v.clear();

    // Bending Membrane Interaction Stiffness Matrix (Pricipal Axis N) 
    Matrix keb_ekn(NumberOfDofs(), NumberOfDofs());
    keb_ekn.clear();

    // Bending Membrane Interaction Stiffness Matrix (Principal Axis V)
    Matrix keb_ekv(NumberOfDofs(), NumberOfDofs());
    keb_ekv.clear();

    // Ineraction Bending Stiffness
    Matrix keb_knkv(NumberOfDofs(), NumberOfDofs());
    keb_knkv.clear();

    // Torsional Stiffness
    Matrix ket_n(NumberOfDofs(), NumberOfDofs()); 
    ket_n.clear();

    // Torsional Stiffness 
    Matrix ket_v(NumberOfDofs(), NumberOfDofs());
    ket_v.clear();

    // Torsional Stiffness
    Matrix ket(NumberOfDofs(), NumberOfDofs()); 
    ket.clear();

    // Declarations 
    Vector3 R1;
    Vector3 R2;
    double A;
    double B;
    Vector3 r1;
    Vector3 r2;
    double a;
    double b;
    R1.clear();
    R2.clear();
    r1.clear();
    r2.clear();

   // Compute the Vectors R1 R2 and the Lentghts A and B in undeformed and deformed state
    ComputeGeometryReference(R1, R2, A, B);
    ComputeGeometryActual(r1, r2, a, b);

    // Debug Check
    IgaDebug::CheckVector(expected_data, "R_1", R1);
    IgaDebug::CheckVector(expected_data, "R_2", R2); 
    IgaDebug::CheckDouble(expected_data, "A", A);
    IgaDebug::CheckDouble(expected_data, "B", B);    
    IgaDebug::CheckVector(expected_data, "r_1", r1);
    IgaDebug::CheckVector(expected_data, "r_2", r2);
    IgaDebug::CheckDouble(expected_data, "a", a);
    IgaDebug::CheckDouble(expected_data, "b", b);

    // get Rotations
    double Phi = GetValue(PHI);
    double Phi_der = GetValue(PHI_0_DER);

    double phi = 0;
    double phi_der = 0;

    ComputePhiReferenceProperty(phi, phi_der);

    IgaDebug::CheckDouble(expected_data, "phi",phi);
    // LOG("[+] phi");   // Debug Check
    IgaDebug::CheckDouble(expected_data, "phi_der",phi_der);
    // LOG("[+] phi_der");   // Debug Check

 
    // Further Declarations
    double B_n;
    double B_v;
    double C_12;
    double C_13;
    double b_n;
    double b_v;
    double c_12;
    double c_13;

    Vector3 N;     // Principal Axis 1 of Cross Section in UNdeformed Config.
    Vector3 V;   // Prinzipal Axis 2 of Cross Section in UNdeformed Config. 
    Vector3 n;   // Principal Axis 1 Of Cross Section in Defomred Config.
    Vector3 v;  // Principal Axis 2 of Cross Section in Deformed Config. 
    Vector3 N0;    // Principal Axis 1 of Cross Section in Reference Config. 
    Vector3 V0;    // Principal Axis 2 of Cross Section in Reference Config. 

    ComputeCrossSectionGeometryReference(R1, R2, N, V, T0_vec, N0, V0, B_n, B_v, C_12, C_13, Phi, Phi_der);

    IgaDebug::CheckVector(expected_data, "N0_reference", N0);
    IgaDebug::CheckVector(expected_data, "V0_reference", V0);
    
    ComputeCrossSectionGeometryActual(R1, R2, r1, r2, N0, V0, N, v, b_n, b_v, c_12, c_13, Phi, Phi_der, phi, phi_der);

    IgaDebug::CheckDouble(expected_data, "b_n", b_n); // checked
    IgaDebug::CheckDouble(expected_data, "b_v", b_v); // checked
    IgaDebug::CheckDouble(expected_data, "C_12", C_12); // Checked
    IgaDebug::CheckDouble(expected_data, "c_12", c_12);
    IgaDebug::CheckDouble(expected_data, "C_13", C_13);  // Checked
    IgaDebug::CheckDouble(expected_data, "c_13", c_13);

    _dL = A;
    double Apow2 = std::pow(A,2);
    double Apow4 = std::pow(A,4);
    double apow2 = std::pow(a,2);

    // Prestress
    //# TODO ### Introduce Presstress 
    double prestress = 0;              // = [var properties] -> [ get_act_Presstress() ];    // Prestress in Normal Direction
    double prestress_bend1 = 0;        // = [var propperties] -> [ get_act_Presstress_bend_n() bzw. get_act_Presstress_bend1() ];      // Prestress arround Z Axis
    double prestress_bend2 = 0;        // [ var propperties ] -> [ get_act_Presstress_bend_v()];     // Prestress arrund the Y Axis
    double prestress_tor = 0;          // var propperties ] -> [ get_act_Presstress_Tor() ];      // Torsional Presstress

    // Set falgs
    // # TODO ### Set Presstress options
    bool prestress_bend1_auto = false;
    bool prestress_bend2_auto = false;
    bool prestesss_tor_auto   = false;

    if (prestress_bend1_auto)
        B_n = 0.00;
    if (prestress_bend2_auto)
        B_v = 0.00;
    if (prestesss_tor_auto){
        C_12 = 0.00;
        C_13 = 0.00;
    }

     // Stresses
     double E11_m = 0.5 * (apow2 - Apow2);      // Green_Lagrange Formulation
     double E11_cur_n = (b_n - B_n);
     double E11_cur_v = (b_v - B_v); 
     double E12       = (c_12 - C_12);
     double E13       = (c_13 - C_13);

    IgaDebug::CheckDouble(expected_data, "E12", E12);


     double S11_m = prestress * _area + E11_m * emod_A / Apow2;         // Normal Force
     double S11_n = prestress_bend1 + E11_cur_n * emod_I_v / Apow2;     // Bending Moment n
     double S11_v = prestress_bend2 + E11_cur_v * emod_I_n / Apow2;     // Bending Moment v
     double S12   = 0.5 * (- prestress_tor + E12 * gmod_It / A);        // 0.5 torsional Moment 
     double S13   = 0.5 * (+ prestress_tor + E13 * gmod_It / A);        // 0.5 torsional Moment

    // 1st Variation
    // Variation of the axial Strain
    Vector eps_dof = ComputeEpsilonFirstDerivative(r1);
    // IgaDebug::CheckVector(expected_data, "EPSILON_DOF", eps_dof);
    // LOG("[+] eps_dof");   // Debug Check

    eps_dof = eps_dof / Apow2;

    // 2nd Variation
    // Variation of axial Strain
    Matrix eps_dof_2 = ComputeEpsilonSecondDerivative(r1);
    // IgaDebug::CheckMatrix(expected_data, "EPSILON_DOF_2", eps_dof_2);
    // LOG("[+] eps_dof_2");   // Debug Check

    eps_dof_2 = eps_dof_2 / Apow4;
    
    // Variation Of Curvature
    Vector curve_dof_n;
    Vector curve_dof_v;
    
    Matrix curve_dof_n_2;
    Matrix curve_dof_v_2;

    // Varation of Torsion
    Vector torsion_dof_n;       // E12
    Vector torsion_dof_v;       // E13
    Matrix torsion_dof_n_2;
    Matrix torsion_dof_v_2;


    ComputeDofNonlinear(curve_dof_n, curve_dof_v, torsion_dof_n, torsion_dof_v, curve_dof_n_2, curve_dof_v_2, torsion_dof_n_2, torsion_dof_v_2, R1, R2, r1, r2, N0, V0, Phi, Phi_der, phi, phi_der);
    
    IgaDebug::CheckVector(expected_data, "curv_dof_n", curve_dof_n);
    IgaDebug::CheckVector(expected_data, "curv_dof_v", curve_dof_v);
    IgaDebug::CheckVector(expected_data, "torsion_dof_n", torsion_dof_n);
    IgaDebug::CheckVector(expected_data, "torsion_dof_v", torsion_dof_v);
    IgaDebug::CheckMatrix(expected_data, "curv_dof_n_2", curve_dof_n_2);
    IgaDebug::CheckMatrix(expected_data, "curv_dof_v_2", curve_dof_v_2);
    IgaDebug::CheckMatrix(expected_data, "tor_dof_n_2", torsion_dof_n_2);
    IgaDebug::CheckMatrix(expected_data, "tor_dof_v_2", torsion_dof_v_2);


    curve_dof_n = curve_dof_n / Apow2;
    curve_dof_v = curve_dof_v / Apow2;
    torsion_dof_n = torsion_dof_n / A;
    torsion_dof_v = torsion_dof_v / A;
    curve_dof_n_2 = curve_dof_n_2 / Apow4;
    curve_dof_v_2 = curve_dof_v_2 / Apow4;
    torsion_dof_n_2 = torsion_dof_n_2 / Apow2;
    torsion_dof_v_2 = torsion_dof_v_2 / Apow2;

    // Stiffness Matrix of the Membran Part 
    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++){
            kem(r,s) = emod_A * eps_dof[r] * eps_dof[s] + eps_dof_2(r,s) * S11_m;
        }
    }
    IgaDebug::CheckMatrix(expected_data, "kem", kem);

    // Stiffness Matrix of the Bending Part
    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++){
            keb_n(r,s) = emod_I_v * curve_dof_n[r] * curve_dof_n[s] + curve_dof_n_2(r,s) * S11_n;
        }
    }
    IgaDebug::CheckMatrix(expected_data, "keb_n", keb_n);

    // Stiffness Matrix of the Bending Part
    IgaDebug::CheckDouble(expected_data, "emod_I_n", emod_I_n);
    IgaDebug::CheckDouble(expected_data, "S11_v", S11_v);

    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++){
            keb_v(r,s) = emod_I_n * curve_dof_v[r] * curve_dof_v[s] + curve_dof_v_2(r,s) * S11_v;
        }
    }
    IgaDebug::CheckMatrix(expected_data, "keb_v", keb_v);

    // Stiffness Matrix of the Torsion Part
    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++){
            ket_n(r,s) = 0.50 * gmod_It * torsion_dof_n[r] * torsion_dof_n[s] + torsion_dof_n_2(r,s) * S12;
        }
    }    
    IgaDebug::CheckDouble(expected_data, "gmod_It", gmod_It);
    IgaDebug::CheckDouble(expected_data, "S12", S12);
    IgaDebug::CheckMatrix(expected_data, "ket_n", ket_n);


    // Stiffness Matrix of the Torsion Part
    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++){
            ket_v(r,s) = 0.50 * gmod_It * torsion_dof_v[r] * torsion_dof_v[s] + torsion_dof_v_2(r,s) * S13;
        }
    }
    IgaDebug::CheckMatrix(expected_data, "ket_v", ket_v);

    // Compute the final Element Stiffness MAtrix
    _gke.clear();
    _gke = kem;
    _gke += keb_n;
    _gke += keb_v;
    _gke += ket_n;
    _gke += ket_v;

    _gfie.clear();
    _gfie  = S11_m * eps_dof;
    _gfie += S11_n * curve_dof_n;
    _gfie += S11_v * curve_dof_v;
    _gfie += S12 * torsion_dof_n;
    _gfie += S13 * torsion_dof_v;

    _gfie = - _gfie;

    IgaDebug::CheckMatrix(expected_data, "stiffness", _gke);
    IgaDebug::CheckVector(expected_data, "external_forces", _gfie);


//     // std::cout << expected_data["t"].GetDouble() << std::endl;
//     // std::cout << expected_data["external_forces"].GetVector() << std::endl;

    KRATOS_CATCH("")
}

//#################################################################################
//#################################################################################
//#
//#                      +++ Compute Geometry Initial +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base Vector
     * 
     * @return
     * 
     * @param[in]   _deriv1          1st Derivative of the Curve in Referens Configuration
     * @param[in]   _deriv2          1st Derivative of the Curve in Referens Configuration 
     * @param[out]  r1              
     * @param[out]  r2              
     * @param[out]  a_ini           Length of the initial tangent Vector
     * @param[out]  b_ini           
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (10/2017)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputeGeometryInitial(
        Vector3& r1,
        Vector3& r2,
        double& a_ini,
        double& b_ini)
{
    r1.resize(3);
    r1.clear();
    r2.resize(3);
    r2.clear();

    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    // Get previous Results
    Vector3 coords;         // Create the Coordinats Vector of the Nodes
    Vector3 tmp_disp_ini;       // Create the initial Displacement Vector of the Nodes
    

    for (size_t i = 0; i < NumberOfNodes(); i++){
        coords = GetGeometry()[i].GetInitialPosition();
        tmp_disp_ini = 0; //# TODO ### Get.Geometry INITIAL DISPLACEMENTS + GetGeometry()[i].FastGetSolutionStepValue(PREDISPLACEMENT);
    
        coords[0] += tmp_disp_ini[0];
        coords[1] += tmp_disp_ini[1];
        coords[2] += tmp_disp_ini[2];      
        
        r1[0] += shape_derivatives(0,i) * coords[0];
        r1[1] += shape_derivatives(0,i) * coords[1];
        r1[2] += shape_derivatives(0,i) * coords[2];

        r2[0] += shape_derivatives(1,i) * coords[0];
        r2[1] += shape_derivatives(1,i) * coords[1];
        r2[2] += shape_derivatives(1,i) * coords[2]; 
    }
    // Set Tollerance if not done bevore
    float tol = 1.0e-8;

    a_ini = norm_2(r1);     // Length of the base Vector

    double tmp = inner_prod(r2, r2) - std::pow( inner_prod(r1, r2), 2) /std::pow(a_ini, 2); 

    // Bending
    if (fabs(tmp) > tol){
         b_ini = std::sqrt(tmp);
    }else{
        b_ini = 0;
    }
}

//#################################################################################
//#################################################################################
//#
//#                      +++ Compute Geometry Initial +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base Vector
     * 
     * @return
     * 
     * @param[in]   _deriv1         1st Derivative of the Curve in Referens Configuration
     * @param[in]   _deriv2         2nd Derivative of the Curve in Referens Configuration 
     * @param[in]   _deriv3         3rd Derivative of the Curve in Referens Configuration 
     * @param[out]  r1              
     * @param[out]  r2              
     * @param[out]  r3              
     * @param[out]  a_ini           Length of the initial tangent Vector
     * @param[out]  b_ini           
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (02/2017)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputeGeometryInitial(
        Vector3& r1,
        Vector3& r2,
        Vector3& r3,
        double& a_ini,
        double& b_ini)
{
    r1.resize(3);
    r1.clear();
    r2.resize(3);
    r2.clear();
    r3.resize(3);
    r3.clear();

    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);    
    
    // Get previous Results
    Vector3 coords;         // Create the Coordinats Vector of the Nodes
    Vector3 tmp_disp_ini;       // Create the initial Displacement Vector of the Nodes
    

    for (size_t i = 0; i < NumberOfNodes(); i++){
        coords = GetGeometry()[i].GetInitialPosition();
        tmp_disp_ini = 0; //# TODO ### Get.Geometry INITIAL DISPLACEMENTS + GetGeometry()[i].FastGetSolutionStepValue(PREDISPLACEMENT);
    
        coords[0] += tmp_disp_ini[0];
        coords[1] += tmp_disp_ini[1];
        coords[2] += tmp_disp_ini[2];      
        
        r1[0] += shape_derivatives(0,i) * coords[0];
        r1[1] += shape_derivatives(0,i) * coords[1];
        r1[2] += shape_derivatives(0,i) * coords[2];

        r2[0] += shape_derivatives(1,i) * coords[0];
        r2[1] += shape_derivatives(1,i) * coords[1];
        r2[2] += shape_derivatives(1,i) * coords[2]; 
        
        r3[0] += shape_derivatives(2,i) * coords[0];
        r3[1] += shape_derivatives(2,i) * coords[1];
        r3[2] += shape_derivatives(2,i) * coords[2]; 
    }
    // Set Tollerance if not done bevore
    float tol = 1.0e-8;

    a_ini = norm_2(r1);     // Length of the base Vector

    double tmp = inner_prod(r2, r2) - std::pow( inner_prod(r1, r2), 2) /std::pow(a_ini, 2); 

    // Bending
    if (fabs(tmp) > tol){
         b_ini = std::sqrt(tmp);
    }else{
        b_ini = 0;
    }
} 

//#################################################################################
//#################################################################################
//#
//#                      +++ Compute_Geometry_Reference +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base Vector
     * 
     * @return
     * 
     * @param[in]   _u_act          NURBS coordinates of integration pont u-direction
     * @param[in]   _deriv1         1st derivatives of the Basis functions at u
     * @param[in]   _deriv2         2nd derivatives of the Basis functions at u
     * @param[out]  R1              1st Derivative of the Curve in Referens Configuration
     * @param[out]  R2              2nd Derivative ot the Curve in Referens Configuration
     * @param[out]  A           Length of the tangent Vector
     * @param[out]  B           
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (10/2014)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputeGeometryReference(
        Vector3& R1,
        Vector3& R2, 
        double& A, 
        double& B)
{
    R1.resize(3);  // Resize 1st Derivative of the Curve
    R2.resize(3);
    R1.clear();     // Clear 1st Derivative of the Curve
    R2.clear();     // Clear 2nd Derivative of the Curve

    // Get the 1st and 2nd Shape Function Deriatides from the ModelPart
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);      

    // Computation of the Basis Functions
    for (size_t i = 0; i < NumberOfNodes(); i++){
       
        const Node<3>& node = GetGeometry()[i];
        
        R1 += shape_derivatives(0,i) * node.GetInitialPosition();
        R2 += shape_derivatives(1,i) * node.GetInitialPosition();
    }

    // Set Tollerance if not done bevore
    float tol = 1.0e-8;

    A = norm_2(R1);     // Length of the base Vector

    double tmp = inner_prod(R2, R2) - std::pow( inner_prod(R1, R2), 2) /std::pow(A, 2); 

    if(fabs(tmp) > tol)
        B = std::sqrt(tmp);
    else 
        B = 0;  
}

//#################################################################################
//#################################################################################
//#
//#                      +++ Compute_Geometry_Reference +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base Vector
     * 
     * @return
     * 
     * @param[in]   _u_act          NURBS coordinates of integration pont u-direction
     * @param[in]   _deriv1         1st derivatives of the Basis functions at u
     * @param[in]   _deriv2         2nd derivatives of the Basis functions at u
     * @param[in]   _deriv3         3rd derivatives of the Basis functions at u
     * @param[out]  R1              1st Derivative of the Curve in Referens Configuration
     * @param[out]  R2              2nd Derivative ot the Curve in Referens Configuration
     * @param[out]  R3              3rd Derivative ot the Curve in Referens Configuration
     * @param[out]  A_ref          Length of the tangent Vector
     * @param[out]  B          
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (02/2018)
     */
//#--------------------------------------------------------------------------------

void IgaBeamElement::ComputeGeometryReference(
        Vector3& R1,
        Vector3& R2,
        Vector3& R3,
        double& A,
        double& B){

    R1.resize(3);
    R1.clear();        // Clears 1st Derivative of the Curve
    R2.resize(3);
    R2.clear();        // Clears 2nd Derivative of the Curve
    R3.resize(3);
    R3.clear();        // Clears 3rd Derivative of the Curve

    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);      

    Vector3 coords;     // Coordinates of the Node 

    for (size_t i = 0; i < NumberOfNodes(); i++){
        const Node<3>& node = GetGeometry()[i];
        
        R1 += shape_derivatives(0,i) * node.GetInitialPosition(); 
        R2 += shape_derivatives(1,i) * node.GetInitialPosition(); 
        R3 += shape_derivatives(2,i) * node.GetInitialPosition(); 
    }

    A = norm_2(R1);       // Length of base Vector A

    double tmp = inner_prod(R2, R2) - std::pow(inner_prod(R1, R2), 2) / std::pow(A, 2);

    // Set Tollerance if not done bevore
    float tol = 1.0e-8;
    
    if(fabs(tmp) > tol)
        B = std::sqrt(tmp);
    else 
        B = 0;      
}

// //#################################################################################
// //#################################################################################
// //#
// //#                      +++ Compute_Geometry_Actual +++
// //#
// //#################################################################################
// //#################################################################################
// //#
// /** Computes the base Vector
//      * 
//      * @return
//      * 
//      * @param[in]   _u_act          NURBS coordinates of integration pont u-direction
//      * @param[in]   _deriv1         1st derivatives of the Basis functions at u
//      * @param[in]   _deriv2         2nd derivatives of the Basis functions at u
//      * @param[in]   _deriv3         3rd derivatives of the Basis functions at u
//      * @param[out]  r1             1st Derivative of the Curve in Referens Configuration
//      * @param[out]  r2             2nd Derivative ot the Curve in Referens Configuration
//      * @param[out]  r3             3rd Derivative ot the Curve in Referens Configuration
//      * @param[out]  a_ref          Length of the tangent Vector
//      * @param[out]  b_ref          
//      * 
//      * @author L.Rauch (10/2018)
//      * 
//      * @note by A.Bauer (10/2014)
//      */
// //#--------------------------------------------------------------------------------

void IgaBeamElement::ComputeGeometryActual(
        Vector3& r1,
        Vector3& r2,
        double& a,
        double& b)
{
    r1.resize(3);
    r1.clear();         // Clear the 1st Derivative of the Curve
    r2.resize(3);
    r2.clear();         // Clear the 2nd Derivative of the Curve

    // Compute the Basis Functions
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);      
  
    // Compute Reference Geometry at each Node
    for (size_t i = 0; i < NumberOfNodes(); i++){
        const Node<3>& node = GetGeometry()[i];
        
        r1 += shape_derivatives(0,i) * node.Coordinates();
        r2 += shape_derivatives(1,i) * node.Coordinates();
    }

    a = norm_2(r1);     // Legth of the Base Vector

    double tmp = inner_prod(r2, r2) - std::pow(inner_prod(r1, r2), 2) / std::pow(a, 2);

    // Set Tollerance if not done bevore
    float tol = 1.0e-8;

    // Bending
    if (fabs(tmp) > tol)
    {
        b = std::sqrt(tmp);
    }else {
        b = 0;
    }
}

//#################################################################################
//#################################################################################
//#
//#                      +++ Compute_Geometry_Actual +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base Vector
     * 
     * @return
     * 
     * @param[in]   _u_act          NURBS coordinates of integration pont u-direction
     * @param[in]   _deriv1         1st derivatives of the Basis functions at u
     * @param[in]   _deriv2         2nd derivatives of the Basis functions at u
     * @param[in]   _deriv3         3rd derivatives of the Basis functions at u
     * @param[out]  r1              1st Derivative of the Curve in Referens Configuration
     * @param[out]  r2              2nd Derivative ot the Curve in Referens Configuration
     * @param[out]  r3              3rd Derivative ot the Curve in Referens Configuration
     * @param[out]  a               Length of the tangent Vector
     * @param[out]  b          
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note by A.Bauer (02/2018)
     */
//#--------------------------------------------------------------------------------

void IgaBeamElement::ComputeGeometryActual(
        Vector3& r1,
        Vector3& r2,
        Vector3& r3,
        double& a,
        double& b)
{    
    r1.resize(3);
    r1.clear();        // Clears 1st Derivative of the Curve
    r2.resize(3);
    r2.clear();        // Clears 2nd Derivative of the Curve
    r3.resize(3);
    r3.clear();        // Clears 3rd Derivative of the Curve

    // Compute the Basis Functions
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);      

    for (size_t i = 0; i < NumberOfNodes(); i++){
        const Node<3>& node = GetGeometry()[i];

        r1 += shape_derivatives(0,i) * node.Coordinates(); 
        r2 += shape_derivatives(1,i) * node.Coordinates();
        r3 += shape_derivatives(2,i) * node.Coordinates(); 
    }

    a = norm_2(r1);       // Length of the Base Vector in Deformed State

    double tmp = inner_prod(r2, r2) - pow(inner_prod(r1, r2), 2) / pow(a, 2);

    // Set Tollerance if not done bevore
    float tol = 1.0e-8;

    // Bendnding
    if (fabs(tmp) > tol)
        b = std::sqrt(tmp);
    else
        b = 0;
}

//#################################################################################
//#################################################################################
//#
//#              +++ Compute Cross Section Geometry Reference +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base vector defined by the property. Is added to input!
     * 
     * @param[in]   _u_act        NURBS coordinates of integration pont u-direction
     * @param[out]  R1            1st Derivative of the Curve
     * @param[out]  R2            2nd Derivative of the Curve
     * @param[out]  R3            3rd Derivative of the Curve
     * @param[out]  A             Length of the Tangent Vector
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note   A.Bauer (10/2014)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputeCrossSectionGeometryReference(
        Vector3 _R1,
        Vector3 _R2,
        Vector3& _n_act,
        Vector3& _v_act,
        Vector3 _T0,
        Vector3& _N0,
        Vector3& _V0,
        double& _B_n,
        double& _B_v,
        double& _C_12,
        double& _C_13,
        double Phi,
        double Phi_der )
{
    // Import Debug data
    auto expected_data = Parameters(GetValue(DEBUG_EXPECTED_DATA));

    _T0 = GetValue(T0);
    _N0 = GetValue(N0);

    double R1_dL = norm_2(_R1);
    BoundedVector<double,3> T_der1;
    BoundedVector<double,3> T0_der1;
    T_der1.clear();
    T0_der1.clear();

    BoundedVector<double,3> T_vec = _R1 / R1_dL;
    T_der1 = _R2 / R1_dL - inner_prod(_R1, _R2) / std::pow(R1_dL,3) * _R1;

    // Compute Matrix Lambda 
    BoundedMatrix<double,3,3> matrix_lambda;
    matrix_lambda.clear();
    comp_lambda(_T0, T_vec, matrix_lambda);
    // LOG("[+](ref) matrix_LAMBDA check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_LAMBDA_REF", matrix_lambda);

    // Compute Matrix Lambda first Derivative
    BoundedMatrix<double,3,3> matrix_lambda_der1;
    matrix_lambda_der1.clear();
    comp_lambda_der(_T0, T0_der1, T_vec, T_der1, matrix_lambda_der1);
    // LOG("[+](ref) matrix_LAMBDA_DER check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_LAMBDA_DER_REF", matrix_lambda_der1);

    // Compute Matrix rodriguez
    BoundedMatrix<double,3,3> matrix_rodriguez;
    matrix_rodriguez.clear();
    comp_rodrigues(T_vec, Phi, matrix_rodriguez);
    // LOG("[+](ref) matrix_RODRIGUEZ check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_RODRIGUEZ_REF", matrix_rodriguez);

    // Compute Matrix rodriguez first Derivative
    BoundedMatrix<double,3,3> matrix_rodriguez_der1; 
    matrix_rodriguez_der1.clear();
    comp_rodrigues_der(T_vec, T_der1, Phi, Phi_der, matrix_rodriguez_der1);
    // LOG("[+](ref) matrix_rodriguez_DERIVATIVE check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_RODRIGUEZ_DER_REF", matrix_rodriguez_der1);

    _n_act.clear();

    // Unit T0 vector (normalisation if not already normalized)
    _T0 = _T0 / norm_2(_T0);

    // Projection in Perpendicular Area
    _N0 = _N0 - inner_prod(_T0, _N0) * _T0;
    _V0 = Cross(_T0, _N0);

    // Normalize the Vectors N and V
    _N0 = _N0 / norm_2(_N0);
    _V0 = _V0 / norm_2(_V0); 

    BoundedVector<double,3> n_tmp;
    n_tmp.clear();

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){

            n_tmp[i] += matrix_lambda(i,j) * _N0[j];
        }
    }
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){

            _n_act[i] += matrix_rodriguez(i,j) * n_tmp[j];
        }
    }

    _n_act = _n_act / norm_2(_n_act);
    _v_act = Cross(T_vec, _n_act);

    BoundedMatrix<double,3,3> matrix_axis1;
    matrix_axis1.clear();

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){

                matrix_axis1(i,j) += matrix_rodriguez_der1(i,k) * matrix_lambda(k,j);
                matrix_axis1(i,j) += matrix_rodriguez(i,k) * matrix_lambda_der1(k,j);
            }
        }
    }

    Vector3 A21;
    Vector3 A31;
    A21.clear();
    A31.clear();

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){

            A21[i] += matrix_axis1(i,j) * _N0[j];
        }
    }
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){

            A31[i] += matrix_axis1(i,j) * _V0[j];
        }
    }
    _B_n  = inner_prod(A21, _R1);
    _B_v  = inner_prod(A31, _R1);
    _C_12 = inner_prod(A31, _n_act);
    _C_13 = inner_prod(A21, _v_act);
}

//#################################################################################
//#################################################################################
//#
//#              +++ Compute Cross Section Geometry Actual +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the base vector defined by the property. Is added to input!
     * 
     * @param[in]   _u_act        
     * @param[out]  R1            
     * @param[out]  R2            
     * @param[out]  R3            
     * @param[out]  A            
     * 
     * @author L.Rauch (11/2018)
     * 
     * @note   A.Bauer (10/2014)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputeCrossSectionGeometryActual(
    Vector3 _R1,
    Vector3 _R2,
    Vector3 _r1,
    Vector3 _r2,
    Vector3 _N0,
    Vector3 _V0,
    Vector3& _n_act,
    Vector3& _v_act,
    double& _b_n,
    double& _b_v,
    double& _c_12,
    double& _c_13,
    double Phi,
    double Phi_der,
    double phi,
    double phi_der)
{
    // Import Debug data
    auto expected_data = Parameters(GetValue(DEBUG_EXPECTED_DATA));

    Vector T0_vec = GetValue(T0);
    // Vector N0_vec = GetValue(N0);

    double R1_dL = norm_2(_R1);
    double r1_dL = norm_2(_r1);
    
    BoundedVector<double,3> T_der1;
    BoundedVector<double,3> t_der1;
    BoundedVector<double,3> T0_der1;
    T_der1.clear();
    t_der1.clear();
    T0_der1.clear();

    BoundedVector<double,3> T_vec = _R1 / R1_dL;
    BoundedVector<double,3> t_vec = _r1 / r1_dL;

    T_der1 = _R2/R1_dL-inner_prod(_R1, _R2)/std::pow(r1_dL,3)*_R1;
    t_der1 = _r2/r1_dL-inner_prod(_r1, _r2)/std::pow(r1_dL,3)*_r1;

    BoundedMatrix<double,3,3> matrix_LAMBDA;
    BoundedMatrix<double,3,3> matrix_lambda;
    BoundedMatrix<double,3,3> matrix_LAMBDA_der1;
    BoundedMatrix<double,3,3> matrix_lambda_der1;
    matrix_LAMBDA.clear();
    matrix_lambda.clear();
    matrix_LAMBDA_der1.clear();
    matrix_lambda_der1.clear();
    BoundedMatrix<double,3,3> matrix_RODRIGUEZ;
    BoundedMatrix<double,3,3> matrix_rodriguez;
    BoundedMatrix<double,3,3> matrix_RODRIGUEZ_der1;
    BoundedMatrix<double,3,3> matrix_rodriguez_der1;
    matrix_RODRIGUEZ.clear();
    matrix_rodriguez.clear();
    matrix_RODRIGUEZ_der1.clear();
    matrix_rodriguez_der1.clear();

    // Compute Matrix Lambda / Lambda_der
    comp_lambda(T0_vec, T_vec, matrix_LAMBDA);
    // LOG("[+](actual) matrix_LAMBDA check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_LAMBDA_ACTUAL", matrix_LAMBDA);
    
    comp_lambda(T_vec,  t_vec, matrix_lambda);
    // LOG("[+](actual) matrix_lambda check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_lambda_ACTUAL", matrix_lambda);

    comp_lambda_der(T0_vec, T0_der1, T_vec, T_der1, matrix_LAMBDA_der1);
    // LOG("[+](actual) matrix_LAMBDA_der check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_LAMBDA_DER_ACTUAL", matrix_LAMBDA_der1);

    comp_lambda_der(T_vec, T_der1,  t_vec,  t_der1, matrix_lambda_der1);
    // LOG("[+](actual) matrix_lambda_der check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_lambda_DER_ACTUAL", matrix_lambda_der1);


    // Compute Matrix Rodriguez / rodriguez_der
    comp_rodrigues(T_vec, Phi, matrix_RODRIGUEZ);
    // LOG("[+](actual) matrix_RODRIGUEZ check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_RODRIGUEZ_ACTUAL", matrix_RODRIGUEZ);

    comp_rodrigues(t_vec, phi, matrix_rodriguez);
    // LOG("[+](actual) matrix_rodriguez check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_rodriguez_ACTUAL", matrix_rodriguez);

    comp_rodrigues_der(T_vec, T_der1, Phi, Phi_der, matrix_RODRIGUEZ_der1);
    // LOG("[+](actual) matrix_RODRIGUEZ_der1 (actual) check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_RODRIGUEZ_DER_ACTUAL", matrix_RODRIGUEZ_der1);

    comp_rodrigues_der(t_vec, t_der1, phi, phi_der, matrix_rodriguez_der1);
    // LOG("[+](actual) matrix_rodriguez_der1 (actual) check");   // Debug Check
    IgaDebug::CheckMatrix(expected_data, "MATRIX_rodriguez_DER_ACTUAL", matrix_rodriguez_der1);
    



    BoundedMatrix<double,3,3> matrix_RODRIGUEZ_LAMBDA;
    BoundedMatrix<double,3,3> matrix_RODRIGUEZ_LAMBDA_der1;
    BoundedMatrix<double,3,3> matrix_RODRIGUEZ_der1_LAMBDA;
    matrix_RODRIGUEZ_LAMBDA.clear();
    matrix_RODRIGUEZ_LAMBDA_der1.clear();
    matrix_RODRIGUEZ_der1_LAMBDA.clear();

    for (int t = 0; t < 3; t++){
        for (int u = 0; u < 3; u++){
            for (int k = 0; k < 3; k++){

                matrix_RODRIGUEZ_LAMBDA(t,u) += matrix_RODRIGUEZ(t,k) * matrix_LAMBDA(k,u);
                matrix_RODRIGUEZ_LAMBDA_der1(t,u) += matrix_RODRIGUEZ(t,k) * matrix_LAMBDA_der1(k,u);
                matrix_RODRIGUEZ_der1_LAMBDA(t,u) += matrix_RODRIGUEZ_der1(t,k) * matrix_LAMBDA(k,u);
            }
        }
    }
    IgaDebug::CheckMatrix(expected_data, "mat_Rod_Lam", matrix_RODRIGUEZ_LAMBDA);
    IgaDebug::CheckMatrix(expected_data, "mat_Rod_Lam_der", matrix_RODRIGUEZ_LAMBDA_der1);
    IgaDebug::CheckMatrix(expected_data, "mat_Rod_der_Lam", matrix_RODRIGUEZ_der1_LAMBDA);


    BoundedMatrix<double,3,3> matrix_lambda_RODRIGUEZ_LAMBDA;
    BoundedMatrix<double,3,3> matrix_lambda_RODRIGUEZ_LAMBDA_der1;
    BoundedMatrix<double,3,3> matrix_lambda_RODRIGUEZ_der1_LAMBDA;
    BoundedMatrix<double,3,3> matrix_lambda_der1_RODRIGUEZ_LAMBDA;
    matrix_lambda_RODRIGUEZ_LAMBDA.clear();
    matrix_lambda_RODRIGUEZ_LAMBDA_der1.clear();
    matrix_lambda_RODRIGUEZ_der1_LAMBDA.clear();
    matrix_lambda_der1_RODRIGUEZ_LAMBDA.clear();

    for (int t = 0; t < 3; t++){
        for (int u = 0; u < 3; u++){
            for (int k = 0; k < 3; k++){

                matrix_lambda_RODRIGUEZ_LAMBDA(t,u)      += matrix_lambda(t,k) * matrix_RODRIGUEZ_LAMBDA(k,u);
                matrix_lambda_RODRIGUEZ_LAMBDA_der1(t,u) += matrix_lambda(t,k) * matrix_RODRIGUEZ_LAMBDA_der1(k,u);
                matrix_lambda_RODRIGUEZ_der1_LAMBDA(t,u) += matrix_lambda(t,k) * matrix_RODRIGUEZ_der1_LAMBDA(k,u);
                matrix_lambda_der1_RODRIGUEZ_LAMBDA(t,u) += matrix_lambda_der1(t,k) * matrix_RODRIGUEZ_LAMBDA(k,u);
            }
        }
    }
    IgaDebug::CheckMatrix(expected_data, "mat_lam_Rod_Lam", matrix_lambda_RODRIGUEZ_LAMBDA);
    IgaDebug::CheckMatrix(expected_data, "mat_lam_Rod_Lam_der", matrix_lambda_RODRIGUEZ_LAMBDA_der1);
    IgaDebug::CheckMatrix(expected_data, "mat_lam_Rod_der_Lam", matrix_lambda_RODRIGUEZ_der1_LAMBDA);
    IgaDebug::CheckMatrix(expected_data, "mat_lam_der_Rod_Lam", matrix_lambda_der1_RODRIGUEZ_LAMBDA);


    BoundedMatrix<double,3,3> matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA;
    BoundedMatrix<double,3,3> matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA_der1;
    BoundedMatrix<double,3,3> matrix_rodriguez_lambda_RODRIGUEZ_der1_LAMBDA;
    BoundedMatrix<double,3,3> matrix_rodriguez_lambda_der1_RODRIGUEZ_LAMBDA;
    BoundedMatrix<double,3,3> matrix_rodriguez_der1_lambda_RODRIGUEZ_LAMBDA;
    matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA.clear();
    matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA_der1.clear();
    matrix_rodriguez_lambda_RODRIGUEZ_der1_LAMBDA.clear();
    matrix_rodriguez_lambda_der1_RODRIGUEZ_LAMBDA.clear();
    matrix_rodriguez_der1_lambda_RODRIGUEZ_LAMBDA.clear();

    for (int t = 0; t < 3; t++){
        for (int u = 0; u < 3; u++){
            for (int k = 0; k < 3; k++){

                matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA(t,u)      += matrix_rodriguez(t,k) 
                                                                    * matrix_lambda_RODRIGUEZ_LAMBDA(k,u);
                                                                    
                matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA_der1(t,u) += matrix_rodriguez(t,k) 
                                                                    * matrix_lambda_RODRIGUEZ_LAMBDA_der1(k,u);
                                                                    
                matrix_rodriguez_lambda_RODRIGUEZ_der1_LAMBDA(t,u) += matrix_rodriguez(t,k)
                                                                    * matrix_lambda_RODRIGUEZ_der1_LAMBDA(k,u);
                
                matrix_rodriguez_lambda_der1_RODRIGUEZ_LAMBDA(t,u) += matrix_rodriguez(t,k)
                                                                    * matrix_lambda_der1_RODRIGUEZ_LAMBDA(k,u);
                                                                    
                matrix_rodriguez_der1_lambda_RODRIGUEZ_LAMBDA(t,u) += matrix_rodriguez_der1(t,k)
                                                                    * matrix_lambda_RODRIGUEZ_LAMBDA(k,u);
            }
        }
    }
    IgaDebug::CheckMatrix(expected_data, "mat_rod_lam_Rod_Lam_der", matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA_der1);
    IgaDebug::CheckMatrix(expected_data, "mat_rod_lam_Rod_der_Lam", matrix_rodriguez_lambda_RODRIGUEZ_der1_LAMBDA);
    IgaDebug::CheckMatrix(expected_data, "mat_rod_lam_der_Rod_Lam", matrix_rodriguez_lambda_der1_RODRIGUEZ_LAMBDA);
    IgaDebug::CheckMatrix(expected_data, "mat_rod_der_lam_Rod_Lam", matrix_rodriguez_der1_lambda_RODRIGUEZ_LAMBDA);
    IgaDebug::CheckMatrix(expected_data, "mat_rod_lam_Rod_Lam", matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA);
   
    BoundedMatrix<double,3,3> matrix_summ_rod_lam_ROD_LAM;
    matrix_summ_rod_lam_ROD_LAM.clear();
    matrix_summ_rod_lam_ROD_LAM = matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA_der1
                                + matrix_rodriguez_lambda_RODRIGUEZ_der1_LAMBDA
                                + matrix_rodriguez_lambda_der1_RODRIGUEZ_LAMBDA
                                + matrix_rodriguez_der1_lambda_RODRIGUEZ_LAMBDA;

    IgaDebug::CheckMatrix(expected_data, "mat_rodlamRodLam_der", matrix_summ_rod_lam_ROD_LAM);
    
    
    Vector3 a21;
    Vector3 a31;
    a21.clear();
    a31.clear();
    _n_act.clear();
    _v_act.clear();

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){

            a21[i] += matrix_summ_rod_lam_ROD_LAM(i,j) * _N0[j];
            a31[i] += matrix_summ_rod_lam_ROD_LAM(i,j) * _V0[j];
            _n_act[i] += matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA(i,j) * _N0[j];
            _v_act[i] += matrix_rodriguez_lambda_RODRIGUEZ_LAMBDA(i,j) * _V0[j];
        }
    }
    IgaDebug::CheckVector(expected_data, "a21", a21);
    IgaDebug::CheckVector(expected_data, "a31", a31);
    IgaDebug::CheckVector(expected_data, "_r1", _r1);
    IgaDebug::CheckVector(expected_data, "_n_act", _n_act);
    IgaDebug::CheckVector(expected_data, "_v_act", _v_act);

    _b_n = inner_prod(a21, _r1);
    _b_v = inner_prod(a31, _r1);
    _c_12 = inner_prod(a31, _n_act);
    _c_13 = inner_prod(a21, _v_act);

    IgaDebug::CheckDouble(expected_data, "c_12", _c_12);
    IgaDebug::CheckDouble(expected_data, "c_13", _c_13);

}



// //#################################################################################
// //#################################################################################
// //#
// //#                          +++ Get Element Results +++
// //#
// //#################################################################################
// //#################################################################################
// //#
// /** Computes the Element results Stored at the Nodes
//      * 
//      * @param[in]   _typ          Nodal Result Tupe
//      * @param[return]   results   returns the Element results
//      * 
//      * @author L.Rauch (10/2018)
//      * 
//      * @note   A.Bauer (10/2014)
//      * @note   Nicht verwendete Funtion? 
//      */
// //#--------------------------------------------------------------------------------
// Vector IgaBeamElement::GetElementResult(
//     node_result_type _type)
// {
//     vector<double> results;
//     return results;
// }


//#################################################################################
//#################################################################################
//#
//#                          +++ Compute Epsilon DOF +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the Variation of the Axial Strain
     * 
     * @param[in]       _r1                         1st Derivative of the Curvature
     * @param[in]       _shape_function_deriv       1st Derivative of the Basis Functions
     * 
     * 
     * @param[return]   Vector with the Variation of the Normal Strain
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note   A.Bauer (10/2014)
     */
//#--------------------------------------------------------------------------------
Vector IgaBeamElement::ComputeEpsilonFirstDerivative(Vector3 _r1)
{
    Vector epsilon_var;
    epsilon_var.resize(NumberOfDofs());
    epsilon_var.clear();
    double r1_L2 = norm_2(_r1);

    // get the degree of Freedom per Node
    std::vector<int> act_dofs; 
    GetDofTypesPerNode(act_dofs);
    int number_dofs_per_node = act_dofs.size();

    Matrix shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    for (int r = 0; r < NumberOfDofs(); r++)
        {
            int xyz_r = r % number_dofs_per_node;
            int i = r / number_dofs_per_node;

            if  (xyz_r > 2)
                epsilon_var(r) = 0;
            else
                epsilon_var(r) = _r1[xyz_r] * shape_derivatives(0,i);
        }
    return epsilon_var;
}

//#################################################################################
//#################################################################################
//#
//#                  +++ Compute Epsilon Second Variation +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the second Variation of the axial Strain
     * 
     * @param[in]       _r1                         1st Derivative of the Curvature
     * 
     * 
     * @param[return]   Vector with the 2nd Variation of the Normal Strain
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note   A.Bauer (03/2015)
     */
//#--------------------------------------------------------------------------------
Matrix IgaBeamElement::ComputeEpsilonSecondDerivative(Vector3 _r1)
{
    Matrix epsilon_var_2;
    epsilon_var_2.resize(NumberOfDofs(), NumberOfDofs());
    epsilon_var_2.clear();

    // get the degree of Freedom per Node
    std::vector<int> act_dofs; 
    GetDofTypesPerNode(act_dofs);
    int number_dofs_per_node = act_dofs.size();

    Matrix shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    for (int r = 0; r < NumberOfDofs(); r++){

        int xyz_r = r % number_dofs_per_node;
        int i = r / number_dofs_per_node;

        if (xyz_r > 2)
        {
            for (int s = 0; s < NumberOfDofs(); s++)
                epsilon_var_2(r,s) = 0.00;
        }
        else    
        {
            for (int s = 0; s < NumberOfDofs(); s++){
                
                int xyz_s = s % number_dofs_per_node;
                int j = s / number_dofs_per_node;

                if (xyz_s > 2)
                    epsilon_var_2(r,s) = 0.00;
                else
                {
                    if (xyz_r == xyz_s)
                        epsilon_var_2(r,s) = shape_derivatives(0,i) * shape_derivatives(0,j);
                    else
                        epsilon_var_2(r,s) = 0.00;
                }
            }
        }   
    }
    return epsilon_var_2;
}

//#################################################################################
//#################################################################################
//#
//#                  +++ Compute Phi Reference Property +++
//#
//#################################################################################
//#################################################################################
//#
/** 
     * 
     * @param[in]   
     * @param[out]  Phi          // Phi Reference
     * @param[out]  Phi_0_der    // Phi Initial
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note   A.Bauer (10/2014)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputePhiReferenceProperty(
        double& phi,
        double& phi_0_der)
{   
    // Construct Vector of Temporarry dislplacements
    double tmp_ini_disp; 
    phi = 0;
    phi_0_der = 0;

    // Get Shape Function Derivatieves from the Element
    Vector& shape_function = GetValue(SHAPE_FUNCTION_VALUES);
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    std::vector<int> act_dofs; 
    GetDofTypesPerNode(act_dofs);
    int number_dofs_per_node = act_dofs.size();

    for (size_t i = 0; i < NumberOfNodes(); i++)
    {
        tmp_ini_disp = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT_ROTATION); 

        phi += shape_function(i) * tmp_ini_disp;
        phi_0_der += shape_derivatives(0,i) * tmp_ini_disp;
    }
}

//#################################################################################
//#################################################################################
//#
//#                             +++ Compute Phi DOF +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the Variation of the Rotation of the Curvature
     * 
     * @param[in]       _func         
     * @param[out]
     * 
     * @author L.Rauch (10/2018)
     * 
     * @note   A.Bauer (06/2014)
     */
//#--------------------------------------------------------------------------------
Vector IgaBeamElement::ComputePhieDof(Vector& _func)
{
    int n_dof = NumberOfDofs();  // * P_Deg; //# TODO ### P_Deg in Kratos
    Vector phi_var;     // Variation of the Axial Strain
    phi_var.resize(n_dof);  //# TODO ### Finktion [ N_Dof in Kratos ]
    phi_var.clear();

    for (int i = 0; i < n_dof; i++)
    {
        int xyz = i%NumberOfDofs(); 
        int r = i/NumberOfDofs(); 

        if (xyz < 2) 
            phi_var[i] = 0;
        else
            phi_var[i] = _func[r];
    }
    return phi_var;
}



//#################################################################################
//#################################################################################
//#
//#                        +++ Compute Dof Nonlinear  +++
//#
//#################################################################################
//#################################################################################
//#
/** Computes the Variation of the Deriative of the Rotation Matrix Lambda (vec1 --> Vec2)
     * 
     * @param[in] 
     * @param[in] 
     * @param[in] 
     * 
     * 
     * @author L.Rauch (11/2018)
     * 
     * @note   A.Bauer (03/2015)
     */
//#--------------------------------------------------------------------------------
void IgaBeamElement::ComputeDofNonlinear(
    Vector& _curve_var_n,
    Vector& _curve_var_v,
    Vector& _tor_var_n,
    Vector& _tor_var_v,
    Matrix& _curve_var_var_n,
    Matrix& _curve_var_var_v,
    Matrix& _tor_var_var_n,
    Matrix& _tor_var_var_v,
    Vector3 _R1,
    Vector3 _R2,
    Vector3 _r1,
    Vector3 _r2,
    Vector3 _N0,
    Vector3 _V0,
    double  _Phi,
    double  _Phi_der,
    double  _phi,
    double  _phi_der)
{

    // Declarations
    Vector3 T_;
    Vector3 t_;
    Vector3 T0_der1;
    Vector3 T_der1;
    Vector3 t_der1;

    T_.clear();
    t_.clear();
    T0_der1.clear();
    T_der1.clear();
    t_der1.clear();

    std::vector<Vector3> t_var1(NumberOfDofs());
    std::vector<Vector3> t_var2(NumberOfDofs() * NumberOfDofs());
    std::vector<Vector3> t_der1var1(NumberOfDofs());
    std::vector<Vector3> t_der1var2(NumberOfDofs() * NumberOfDofs());
    BoundedMatrix<double, 3, 3>  matrix_lambda;
    BoundedMatrix<double, 3, 3>  matrix_lambda_der1;
    std::vector<BoundedMatrix<double, 3, 3>>  matrix_lambda_var1(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>>  matrix_lambda_var2(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>>  matrix_lambda_der1var1(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>>  matrix_lambda_der1var2(NumberOfDofs() * NumberOfDofs());
    BoundedMatrix<double, 3, 3>  matrix_LAMBDA;
    BoundedMatrix<double, 3, 3>  matrix_LAMBDA_der1;
    BoundedMatrix<double, 3, 3>  matrix_rodriguez;
    BoundedMatrix<double, 3, 3>  matrix_rodriguez_der1;
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rodriguez_var1(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rodriguez_var2(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rodriguez_der1var1(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rodriguez_der1var2(NumberOfDofs() * NumberOfDofs());
    BoundedMatrix<double, 3, 3>  matrix_RODRIGUEZ;
    BoundedMatrix<double, 3, 3>  matrix_RODRIGUEZ_der1;

    // t_var1.clear();
    // t_var2.clear();
    // t_der1var1.clear();
    // t_der1var2.clear();
    matrix_lambda.clear();
    matrix_lambda_der1.clear();
    // matrix_lambda_var1.clear();
    // matrix_lambda_var2.clear();
    // matrix_lambda_der1var1.clear();
    // matrix_lambda_der1var2.clear();
    matrix_LAMBDA.clear();
    matrix_LAMBDA_der1.clear();
    matrix_rodriguez.clear();
    matrix_rodriguez_der1.clear();
    // matrix_rodriguez_var1.clear();
    // matrix_rodriguez_var2.clear();
    // matrix_rodriguez_der1var1.clear();
    // matrix_rodriguez_der1var2.clear();
    matrix_RODRIGUEZ.clear();
    matrix_RODRIGUEZ_der1.clear();

    t_ = _r1 / norm_2(_r1);
    T_ = _R1 / norm_2(_R1);
    t_der1 = _r2 / norm_2(_r1) - inner_prod(_r1, _r2) / std::pow(norm_2(_r1), 3) * _r1;
    T_der1 = _R2 / norm_2(_R1) - inner_prod(_R1, _R2) / std::pow(norm_2(_R1), 3) * _R1;


    
    comp_T_var(_r1, t_var1);
    comp_T_der_var(_r1,_r2, t_der1var1);
    comp_T_var_var(_r1, t_var2);
    comp_T_der_var_var(_r1,_r2, t_der1var2);

    auto expected_data = Parameters(GetValue(DEBUG_EXPECTED_DATA));
    // IgaDebug::CheckVectorVar(expected_data, "T_VAR", t_var1);
    // IgaDebug::CheckVectorVar(expected_data, "T_DER_VAR", t_der1var1);
    // IgaDebug::CheckVectorVarVar(expected_data, "T_VAR_VAR", t_var2);
    // IgaDebug::CheckVectorVarVar(expected_data, "T_DER_VAR_VAR", t_der1var2);

    comp_rodrigues(t_, _phi, matrix_rodriguez);
    comp_rodrigues_der(t_, t_der1, _phi, _phi_der, matrix_rodriguez_der1);
    comp_rodrigues_der_var(t_, t_var1, t_der1, t_der1var1, _phi, _phi_der, matrix_rodriguez_der1var1);
    comp_rodrigues_der_var_var(t_, t_var1, t_var2, t_der1, t_der1var1, t_der1var2, _phi, _phi_der, matrix_rodriguez_der1var2);
    comp_rodrigues_var(t_, t_var1, _phi, matrix_rodriguez_var1);
    comp_rodrigues_var_var(t_, t_var1, t_var2, _phi, matrix_rodriguez_var2);
    comp_rodrigues(T_, _Phi, matrix_RODRIGUEZ);
    comp_rodrigues_der(T_, T_der1, _Phi, _Phi_der, matrix_RODRIGUEZ_der1);

    // IgaDebug::CheckMatrix(expected_data, "MAT_rodriguez", matrix_rodriguez);
    // IgaDebug::CheckMatrix(expected_data, "MAT_rodriguez_DER", matrix_rodriguez_der1);
    // IgaDebug::CheckMatrixVar(expected_data, "MAT_rodriguez_DER_VAR", matrix_rodriguez_der1var1);
    // IgaDebug::CheckMatrixVarVar(expected_data, "MAT_rodriguez_DER_VAR_VAR", matrix_rodriguez_der1var2);
    // IgaDebug::CheckMatrixVar(expected_data, "MAT_rodriguez_VAR", matrix_rodriguez_var1);
    // IgaDebug::CheckMatrixVarVar(expected_data, "MAT_rodriguez_VAR_VAR", matrix_rodriguez_var2);
    // IgaDebug::CheckMatrix(expected_data, "MAT_RODRIGUEZ", matrix_RODRIGUEZ);
    // IgaDebug::CheckMatrix(expected_data, "MAT_RODRIGUEZ_DER", matrix_RODRIGUEZ_der1);

    // Get Axis Value t0_0
    Vector3 T_0 = GetValue(T0);
    
    comp_lambda(T_, t_, matrix_lambda);
    comp_lambda(T_0, T_, matrix_LAMBDA);
    comp_lambda_der(T_, T_der1, t_, t_der1, matrix_lambda_der1);
    comp_lambda_der(T_0, T0_der1, T_ , T_der1, matrix_LAMBDA_der1);
    comp_lambda_var(T_, t_, t_var1, matrix_lambda_var1);
    comp_lambda_der_var(T_, T_der1, t_, t_var1, t_der1, t_der1var1, matrix_lambda_der1var1);
    comp_lambda_var_var(T_, t_, t_var1, t_var2, matrix_lambda_var2);
    comp_lambda_der_var_var(T_, T_der1, t_, t_var1, t_var2, t_der1, t_der1var1, t_der1var2, matrix_lambda_der1var2);
    
    // IgaDebug::CheckMatrix(expected_data, "MAT_lambda", matrix_lambda);
    // IgaDebug::CheckMatrix(expected_data, "MAT_LAMBDA", matrix_LAMBDA);
    // IgaDebug::CheckMatrix(expected_data, "MAT_lambda_DER", matrix_lambda_der1);
    // IgaDebug::CheckMatrix(expected_data, "MAT_LAMBDA_DER", matrix_LAMBDA_der1);
    // IgaDebug::CheckMatrixVar(expected_data, "MAT_lambda_VAR", matrix_lambda_var1);
    // IgaDebug::CheckMatrixVar(expected_data, "MAT_lambda_DER_VAR", matrix_lambda_der1var1);
    // IgaDebug::CheckMatrixVarVar(expected_data, "MAT_lambda_VAR_VAR", matrix_lambda_var2);
    // IgaDebug::CheckMatrixVarVar(expected_data, "MAT_lambda_DER_VAR_VAR", matrix_lambda_der1var2);


    // Configuration of A_i,1 --> _der
    BoundedMatrix<double,3,3> matrix_Rod_Lam_der;
    BoundedMatrix<double,3,3> matrix_Rod_der_Lam;
    BoundedMatrix<double,3,3> matrix_Rod_Lam;
    BoundedMatrix<double,3,3> matrix_RodLam_der;

    matrix_Rod_Lam_der.clear();
    matrix_Rod_der_Lam.clear();
    matrix_Rod_Lam.clear();
    matrix_RodLam_der.clear();

    for (size_t t = 0; t < 3; t++){
        for (size_t u = 0; u < 3; u++){
            for (size_t k = 0; k < 3; k++){

                matrix_Rod_Lam(t,u)     += matrix_RODRIGUEZ(t,k) * matrix_LAMBDA(k,u);
                matrix_Rod_Lam_der(t,u) += matrix_RODRIGUEZ(t,k) * matrix_LAMBDA_der1(k,u);
                matrix_Rod_der_Lam(t,u) += matrix_RODRIGUEZ_der1(t,k) * matrix_LAMBDA(k,u);
            }
        }
    }
    matrix_RodLam_der = matrix_Rod_Lam_der + matrix_Rod_der_Lam;
    // IgaDebug::CheckMatrix(expected_data, "mat_RodLam_der", matrix_RodLam_der);

    BoundedMatrix<double,3,3> matrix_lam_Rod_Lam_der;
    BoundedMatrix<double,3,3> matrix_lam_Rod_der_Lam;
    BoundedMatrix<double,3,3> matrix_lam_Rod_Lam;
    BoundedMatrix<double,3,3> matrix_lam_der_Rod_Lam;
    BoundedMatrix<double,3,3> matrix_lamRodLam_der;

     matrix_lam_Rod_Lam_der.clear();
     matrix_lam_Rod_der_Lam.clear();
     matrix_lam_Rod_Lam.clear();
     matrix_lam_der_Rod_Lam.clear();
     matrix_lamRodLam_der.clear();

    for (size_t t = 0; t < 3; t++){
        for (size_t u = 0; u < 3; u++){
            for (size_t k = 0; k < 3; k++){

                matrix_lam_Rod_Lam(t,u)     += matrix_lambda(t,k) * matrix_Rod_Lam(k,u);
                matrix_lam_Rod_Lam_der(t,u) += matrix_lambda(t,k) * matrix_Rod_Lam_der(k,u);
                matrix_lam_Rod_der_Lam(t,u) += matrix_lambda(t,k) * matrix_Rod_der_Lam(k,u);
                matrix_lam_der_Rod_Lam(t,u) += matrix_lambda_der1(t,k) * matrix_Rod_Lam(k,u);
            }
        }
    }
    matrix_lamRodLam_der    = matrix_lam_Rod_Lam_der 
                            + matrix_lam_Rod_der_Lam 
                            + matrix_lam_der_Rod_Lam;
    // IgaDebug::CheckMatrix(expected_data, "mat_lamRodLam_der", matrix_lamRodLam_der);

    BoundedMatrix<double,3,3> matrix_rod_lam_Rod_Lam_der;
    BoundedMatrix<double,3,3> matrix_rod_lam_Rod_der_Lam;
    BoundedMatrix<double,3,3> matrix_rod_lam_der_Rod_Lam;
    BoundedMatrix<double,3,3> matrix_rod_der_lam_Rod_Lam;
    BoundedMatrix<double,3,3> matrix_rod_lam_Rod_Lam;
    BoundedMatrix<double,3,3> matrix_rodlamRodLam_der;

    matrix_rod_lam_Rod_Lam_der.clear(); 
    matrix_rod_lam_Rod_der_Lam.clear(); 
    matrix_rod_lam_der_Rod_Lam.clear(); 
    matrix_rod_der_lam_Rod_Lam.clear(); 
    matrix_rod_lam_Rod_Lam.clear(); 
    matrix_rodlamRodLam_der.clear();

    for (size_t t = 0; t < 3; t++){
        for (size_t u = 0; u < 3; u++){
            for (size_t k = 0; k < 3; k++){

                matrix_rod_lam_Rod_Lam(t,u)     += matrix_rodriguez(t,k) * matrix_lam_Rod_Lam(k,u);
                matrix_rod_lam_Rod_Lam_der(t,u) += matrix_rodriguez(t,k) * matrix_lam_Rod_Lam_der(k,u);
                matrix_rod_lam_Rod_der_Lam(t,u) += matrix_rodriguez(t,k) * matrix_lam_Rod_der_Lam(k,u);
                matrix_rod_lam_der_Rod_Lam(t,u) += matrix_rodriguez(t,k) * matrix_lam_der_Rod_Lam(k,u);
                matrix_rod_der_lam_Rod_Lam(t,u) += matrix_rodriguez_der1(t,k) * matrix_lam_Rod_Lam(k,u);
            }
        }
    }
    matrix_rodlamRodLam_der = matrix_rod_lam_Rod_Lam_der 
                            + matrix_rod_lam_Rod_der_Lam
                            + matrix_rod_lam_der_Rod_Lam
                            + matrix_rod_der_lam_Rod_Lam;

    // IgaDebug::CheckMatrix(expected_data, "mat_rodlamRodLam_der", matrix_rodlamRodLam_der);
    

    // Variation of A_i,1 --> _der_var

    std::vector<BoundedMatrix<double, 3, 3>> matrix_lam_var_Rod_Lam_der(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_lam_var_Rod_der_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_lam_der_var_Rod_Lam(NumberOfDofs()); 
    std::vector<BoundedMatrix<double, 3, 3>> matrix_lam_var_Rod_Lam(NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r++)
    {
        if (GetDofTypeIndex(r) != 3) 
        {
            matrix_lam_var_Rod_Lam[r]     = prod(matrix_lambda_var1[r], matrix_Rod_Lam);
            matrix_lam_var_Rod_Lam_der[r] = prod(matrix_lambda_var1[r], matrix_Rod_Lam_der);
            matrix_lam_var_Rod_der_Lam[r] = prod(matrix_lambda_var1[r], matrix_Rod_der_Lam);
            matrix_lam_der_var_Rod_Lam[r] = prod(matrix_lambda_der1var1[r], matrix_Rod_Lam);
        }
        else
        {
            matrix_lam_var_Rod_Lam[r].clear();
            matrix_lam_var_Rod_Lam_der[r].clear();
            matrix_lam_var_Rod_der_Lam[r].clear();
            matrix_lam_der_var_Rod_Lam[r].clear();
        }
    }

    // IgaDebug::CheckMatrixVar(expected_data, "mat_lam_var_Rod_Lam", matrix_lam_var_Rod_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_lam_var_Rod_Lam_der", matrix_lam_var_Rod_Lam_der);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_lam_var_Rod_der_Lam", matrix_lam_var_Rod_der_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_lam_der_var_Rod_Lam", matrix_lam_der_var_Rod_Lam);

    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_var_lam_Rod_Lam_der(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_var_lam_Rod_der_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_var_lam_der_Rod_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_der_var_lam_Rod_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_der_lam_var_Rod_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_lam_der_var_Rod_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_lam_var_Rod_der_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_lam_var_Rod_Lam_der(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_lam_var_Rod_Lam(NumberOfDofs());
    std::vector<BoundedMatrix<double, 3, 3>> matrix_rod_var_lam_Rod_Lam(NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r++)
    {
            matrix_rod_var_lam_Rod_Lam_der[r] = prod(matrix_rodriguez_var1[r], matrix_lam_Rod_Lam_der);
            matrix_rod_var_lam_Rod_der_Lam[r] = prod(matrix_rodriguez_var1[r], matrix_lam_Rod_der_Lam);
            matrix_rod_var_lam_der_Rod_Lam[r] = prod(matrix_rodriguez_var1[r], matrix_lam_der_Rod_Lam);
            matrix_rod_der_var_lam_Rod_Lam[r] = prod(matrix_rodriguez_der1var1[r], matrix_lam_Rod_Lam);
            matrix_rod_der_lam_var_Rod_Lam[r] = prod(matrix_rodriguez_der1, matrix_lam_var_Rod_Lam[r]);
            matrix_rod_lam_der_var_Rod_Lam[r] = prod(matrix_rodriguez, matrix_lam_der_var_Rod_Lam[r]);
            matrix_rod_lam_var_Rod_der_Lam[r] = prod(matrix_rodriguez, matrix_lam_var_Rod_der_Lam[r]);
            matrix_rod_lam_var_Rod_Lam_der[r] = prod(matrix_rodriguez, matrix_lam_var_Rod_Lam_der[r]);
            matrix_rod_lam_var_Rod_Lam[r]     = prod(matrix_rodriguez, matrix_lam_var_Rod_Lam[r]);
            matrix_rod_var_lam_Rod_Lam[r]     = prod(matrix_rodriguez_var1[r], matrix_lam_Rod_Lam);
    }
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_var_lam_Rod_Lam_der", matrix_rod_var_lam_Rod_Lam_der);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_var_lam_Rod_der_Lam", matrix_rod_var_lam_Rod_der_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_var_lam_der_Rod_Lam", matrix_rod_var_lam_der_Rod_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_der_var_lam_Rod_Lam", matrix_rod_der_var_lam_Rod_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_der_lam_var_Rod_Lam", matrix_rod_der_lam_var_Rod_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_lam_der_var_Rod_Lam", matrix_rod_lam_der_var_Rod_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_lam_var_Rod_der_Lam", matrix_rod_lam_var_Rod_der_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_lam_var_Rod_Lam_der", matrix_rod_lam_var_Rod_Lam_der);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_lam_var_Rod_Lam", matrix_rod_lam_var_Rod_Lam);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rod_var_lam_Rod_Lam", matrix_rod_var_lam_Rod_Lam);

    std::vector<BoundedMatrix<double,3,3>>  matrix_rodlamRodLam_der_var(NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rodlamRodLam_var(NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r++)
    {   
    matrix_rodlamRodLam_der_var[r]  = matrix_rod_var_lam_Rod_Lam_der[r]
                                    + matrix_rod_var_lam_Rod_der_Lam[r]
                                    + matrix_rod_var_lam_der_Rod_Lam[r]
                                    + matrix_rod_der_var_lam_Rod_Lam[r]
                                    + matrix_rod_der_lam_var_Rod_Lam[r]
                                    + matrix_rod_lam_der_var_Rod_Lam[r]
                                    + matrix_rod_lam_var_Rod_der_Lam[r]
                                    + matrix_rod_lam_var_Rod_Lam_der[r];
    matrix_rodlamRodLam_var[r]  = matrix_rod_lam_var_Rod_Lam[r] + matrix_rod_var_lam_Rod_Lam[r];
    }

    // IgaDebug::CheckMatrixVar(expected_data, "mat_rodlamRodLam_der_var", matrix_rodlamRodLam_der_var);
    // IgaDebug::CheckMatrixVar(expected_data, "mat_rodlamRodLam_var", matrix_rodlamRodLam_var);


    // 2nd variation of A_i,1 ->_der_var_var

    std::vector<BoundedMatrix<double,3,3>> matrix_lam_var_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_lam_var_var_Rod_der_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_lam_var_var_Rod_Lam_der(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_lam_der_var_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++)
        {
        matrix_lam_var_var_Rod_Lam[r * NumberOfDofs() +s]     = prod(matrix_lambda_var2[r * NumberOfDofs() +s], matrix_Rod_Lam);
        matrix_lam_var_var_Rod_der_Lam[r * NumberOfDofs() +s] = prod(matrix_lambda_var2[r * NumberOfDofs() +s], matrix_Rod_der_Lam);
        matrix_lam_var_var_Rod_Lam_der[r * NumberOfDofs() +s] = prod(matrix_lambda_var2[r * NumberOfDofs() +s], matrix_Rod_Lam_der);
        matrix_lam_der_var_var_Rod_Lam[r * NumberOfDofs() +s] = prod(matrix_lambda_der1var2[r * NumberOfDofs() +s], matrix_Rod_Lam);
        }
    }

    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_lam_var_var_Rod_Lam", matrix_lam_var_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_lam_var_var_Rod_der_Lam", matrix_lam_var_var_Rod_der_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_lam_var_var_Rod_Lam_der", matrix_lam_var_var_Rod_Lam_der);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_lam_der_var_var_Rod_Lam", matrix_lam_der_var_var_Rod_Lam);

    std::vector<BoundedMatrix<double,3,3>> matrix_rod_der_lam_var_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_lam_der_var_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_lam_var_var_Rod_der_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_lam_var_var_Rod_Lam_der(NumberOfDofs() * NumberOfDofs());
    
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_der_var_lam_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_lam_der_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_lam_var_Rod_der_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_lam_var_Rod_Lam_der(NumberOfDofs() * NumberOfDofs());

    std::vector<BoundedMatrix<double,3,3>> matrix_rod_der_var_var_lam_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_var_lam_der_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_var_lam_Rod_der_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_var_lam_Rod_Lam_der(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_lam_var_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_lam_var_Rod_Lam(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rod_var_var_lam_Rod_Lam(NumberOfDofs() * NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r ++){

        for (size_t s = 0; s != NumberOfDofs(); s++){

        matrix_rod_der_lam_var_var_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_der1, matrix_lam_var_var_Rod_Lam[r * NumberOfDofs() + s]);
        matrix_rod_lam_der_var_var_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez, matrix_lam_der_var_var_Rod_Lam[r * NumberOfDofs() + s]);
        matrix_rod_lam_var_var_Rod_der_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez, matrix_lam_var_var_Rod_der_Lam[r * NumberOfDofs() + s]);
        matrix_rod_lam_var_var_Rod_Lam_der[r * NumberOfDofs() + s] = prod(matrix_rodriguez, matrix_lam_var_var_Rod_Lam_der[r * NumberOfDofs() + s]);

        matrix_rod_der_var_lam_var_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_der1var1[r], matrix_lam_var_Rod_Lam[s]);
        matrix_rod_var_lam_der_var_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var1[r], matrix_lam_der_var_Rod_Lam[s]);
        matrix_rod_var_lam_var_Rod_der_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var1[r], matrix_lam_var_Rod_der_Lam[s]);
        matrix_rod_var_lam_var_Rod_Lam_der[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var1[r], matrix_lam_var_Rod_Lam_der[s]);

        matrix_rod_der_var_var_lam_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_der1var2[r * NumberOfDofs() + s], matrix_lam_Rod_Lam);
        matrix_rod_var_var_lam_der_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var2[r * NumberOfDofs() + s], matrix_lam_der_Rod_Lam);
        matrix_rod_var_var_lam_Rod_der_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var2[r * NumberOfDofs() + s], matrix_lam_Rod_der_Lam);
        matrix_rod_var_var_lam_Rod_Lam_der[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var2[r * NumberOfDofs() + s], matrix_lam_Rod_Lam_der);
        matrix_rod_lam_var_var_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez, matrix_lam_var_var_Rod_Lam[r * NumberOfDofs() + s]);
        matrix_rod_var_lam_var_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var1[r], matrix_lam_var_Rod_Lam[s]);
        matrix_rod_var_var_lam_Rod_Lam[r * NumberOfDofs() + s] = prod(matrix_rodriguez_var2[r * NumberOfDofs() + s], matrix_lam_Rod_Lam);
        }
    }
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_der_lam_var_var_Rod_Lam", matrix_rod_der_lam_var_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_lam_der_var_var_Rod_Lam", matrix_rod_lam_der_var_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_lam_var_var_Rod_der_Lam", matrix_rod_lam_var_var_Rod_der_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_lam_var_var_Rod_Lam_der", matrix_rod_lam_var_var_Rod_Lam_der);

    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_der_var_lam_var_Rod_Lam", matrix_rod_der_var_lam_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_lam_der_var_Rod_Lam", matrix_rod_var_lam_der_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_lam_var_Rod_der_Lam", matrix_rod_var_lam_var_Rod_der_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_lam_var_Rod_Lam_der", matrix_rod_var_lam_var_Rod_Lam_der);

    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_der_var_var_lam_Rod_Lam", matrix_rod_der_var_var_lam_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_var_lam_der_Rod_Lam", matrix_rod_var_var_lam_der_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_var_lam_Rod_der_Lam", matrix_rod_var_var_lam_Rod_der_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_var_lam_Rod_Lam_der", matrix_rod_var_var_lam_Rod_Lam_der);

    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_lam_var_var_Rod_Lam", matrix_rod_lam_var_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_lam_var_Rod_Lam", matrix_rod_var_lam_var_Rod_Lam);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rod_var_var_lam_Rod_Lam", matrix_rod_var_var_lam_Rod_Lam);

    
    Vector3 vec_n;
    Vector3 vec_v;

    vec_n = prod(matrix_rod_lam_Rod_Lam, _N0);
    vec_v = prod(matrix_rod_lam_Rod_Lam, _V0);
    // IgaDebug::CheckVector(expected_data, "vec_n", vec_n);
    // IgaDebug::CheckVector(expected_data, "vec_v", vec_v);

    std::vector<Vector3> vec_n_var(NumberOfDofs());
    std::vector<Vector3> vec_v_var(NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r++)
    {
        vec_n_var[r] = prod(matrix_rodlamRodLam_var[r], _N0);
        vec_v_var[r] = prod(matrix_rodlamRodLam_var[r], _V0);
    }
    // IgaDebug::CheckVectorVar(expected_data, "vec_n_var", vec_n_var);
    // IgaDebug::CheckVectorVar(expected_data, "vec_v_var", vec_v_var);

    std::vector<BoundedMatrix<double,3,3>> matrix_rodlamRodLam_var_var(NumberOfDofs() * NumberOfDofs());
    std::vector<BoundedMatrix<double,3,3>> matrix_rodlamRodLam_der_var_var(NumberOfDofs() * NumberOfDofs());


    for (size_t r = 0; r != NumberOfDofs(); r ++){
        for (size_t s = 0; s != NumberOfDofs(); s++){

            matrix_rodlamRodLam_var_var[r * NumberOfDofs() + s]     = matrix_rod_lam_var_var_Rod_Lam[r * NumberOfDofs() +s] 
                                                                    + matrix_rod_var_lam_var_Rod_Lam[r * NumberOfDofs() +s]
                                                                    + matrix_rod_var_lam_var_Rod_Lam[s * NumberOfDofs() +r]
                                                                    + matrix_rod_var_var_lam_Rod_Lam[r * NumberOfDofs() +s];

            matrix_rodlamRodLam_der_var_var[r * NumberOfDofs() + s] = matrix_rod_der_lam_var_var_Rod_Lam[r * NumberOfDofs() + s]
                                                                    + matrix_rod_lam_der_var_var_Rod_Lam[r * NumberOfDofs() + s]
                                                                    + matrix_rod_lam_var_var_Rod_der_Lam[r * NumberOfDofs() + s]
                                                                    + matrix_rod_lam_var_var_Rod_Lam_der[r * NumberOfDofs() + s]
                                                                
                                                                    + matrix_rod_der_var_lam_var_Rod_Lam[r * NumberOfDofs() + s]
                                                                    + matrix_rod_var_lam_der_var_Rod_Lam[r * NumberOfDofs() + s]
                                                                    + matrix_rod_var_lam_var_Rod_der_Lam[r * NumberOfDofs() + s]
                                                                    + matrix_rod_var_lam_var_Rod_Lam_der[r * NumberOfDofs() + s]    
                                                                
                                                                    + matrix_rod_der_var_lam_var_Rod_Lam[s * NumberOfDofs() + r]
                                                                    + matrix_rod_var_lam_der_var_Rod_Lam[s * NumberOfDofs() + r]
                                                                    + matrix_rod_var_lam_var_Rod_der_Lam[s * NumberOfDofs() + r]
                                                                    + matrix_rod_var_lam_var_Rod_Lam_der[s * NumberOfDofs() + r]

                                                                    + matrix_rod_der_var_var_lam_Rod_Lam[s * NumberOfDofs() + r]
                                                                    + matrix_rod_var_var_lam_der_Rod_Lam[s * NumberOfDofs() + r]
                                                                    + matrix_rod_var_var_lam_Rod_der_Lam[s * NumberOfDofs() + r]
                                                                    + matrix_rod_var_var_lam_Rod_Lam_der[s * NumberOfDofs() + r];
        }
    }
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rodlamRodLam_var_var", matrix_rodlamRodLam_var_var);
    // IgaDebug::CheckMatrixVarVar(expected_data, "mat_rodlamRodLam_der_var_var", matrix_rodlamRodLam_der_var_var);

    std::vector<Vector3> vec_n_var_var(NumberOfDofs() * NumberOfDofs());
    std::vector<Vector3> vec_v_var_var(NumberOfDofs() * NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); r++){
        for (size_t s = 0; s != NumberOfDofs(); s++)
        {
            vec_n_var_var[r * NumberOfDofs() + s] = prod(matrix_rodlamRodLam_var_var[r * NumberOfDofs() + s], _N0);
            vec_v_var_var[r * NumberOfDofs() + s] = prod(matrix_rodlamRodLam_var_var[r * NumberOfDofs() + s], _V0);
        }
    }
    // IgaDebug::CheckVectorVarVar(expected_data, "vec_n_var_var", vec_n_var_var);
    // IgaDebug::CheckVectorVarVar(expected_data, "vec_v_var_var", vec_v_var_var);

    std::vector<Vector3> r1_var(NumberOfDofs());

    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) { 
        for (size_t t = 0; t != 3; ++t) {
            size_t xyz = GetDofTypeIndex(r);
            size_t i = GetShapeIndex(r);
            if (t == xyz) {
                r1_var[r][t] = shape_derivatives(0, i);
            } else {
                r1_var[r][t] = 0;
            }
        }
    }

    _curve_var_n.resize(NumberOfDofs());
    _curve_var_v.resize(NumberOfDofs());
    _curve_var_var_n.resize(NumberOfDofs(), NumberOfDofs());
    _curve_var_var_v.resize(NumberOfDofs(), NumberOfDofs());
    _tor_var_n.resize(NumberOfDofs());
    _tor_var_v.resize(NumberOfDofs());
    _tor_var_var_n.resize(NumberOfDofs(), NumberOfDofs());
    _tor_var_var_v.resize(NumberOfDofs(), NumberOfDofs());
   
    for (size_t r = 0; r != NumberOfDofs(); r++)
    {
        _curve_var_n[r] = inner_prod(prod(matrix_rodlamRodLam_der_var[r], _N0), _r1) + inner_prod(prod(matrix_rodlamRodLam_der, _N0), r1_var[r]);
        _curve_var_v[r] = inner_prod(prod(matrix_rodlamRodLam_der_var[r], _V0), _r1) + inner_prod(prod(matrix_rodlamRodLam_der, _V0), r1_var[r]);
        _tor_var_n[r]   = inner_prod(prod(matrix_rodlamRodLam_der_var[r], _V0), vec_n) + inner_prod(prod(matrix_rodlamRodLam_der, _V0), vec_n_var[r]);
        _tor_var_v[r]   = inner_prod(prod(matrix_rodlamRodLam_der_var[r], _N0), vec_v) + inner_prod(prod(matrix_rodlamRodLam_der, _N0), vec_v_var[r]);

        for (size_t s = 0; s != NumberOfDofs(); s++)
        {
            _curve_var_var_n(r,s)   = inner_prod(prod(matrix_rodlamRodLam_der_var_var[r * NumberOfDofs() + s], _N0), _r1)
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[r], _N0), r1_var[s])
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[s], _N0), r1_var[r]);
                                  
            _curve_var_var_v(r,s)   = inner_prod(prod(matrix_rodlamRodLam_der_var_var[r * NumberOfDofs() + s], _V0), _r1)
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[r], _V0), r1_var[s])
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[s], _V0), r1_var[r]);

            _tor_var_var_n(r,s)     = inner_prod(prod(matrix_rodlamRodLam_der_var_var[r * NumberOfDofs() + s], _V0), vec_n)
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[r], _V0), vec_n_var[s])
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[s], _V0), vec_n_var[r])
                                    + inner_prod(prod(matrix_rodlamRodLam_der, _V0), vec_n_var_var[r * NumberOfDofs() + s]);
            
            _tor_var_var_v(r,s)     = inner_prod(prod(matrix_rodlamRodLam_der_var_var[r * NumberOfDofs() + s], _N0), vec_v)
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[r], _N0), vec_v_var[s])
                                    + inner_prod(prod(matrix_rodlamRodLam_der_var[s], _N0), vec_v_var[r])
                                    + inner_prod(prod(matrix_rodlamRodLam_der, _N0), vec_v_var_var[r * NumberOfDofs() + s]);
        }
    }
    // IgaDebug::CheckVector(expected_data, "_cur_var_n", _curve_var_n);
    // IgaDebug::CheckVector(expected_data, "_cur_var_v", _curve_var_v);
    // IgaDebug::CheckVector(expected_data, "_tor_var_n", _tor_var_n);
    // IgaDebug::CheckVector(expected_data, "_tor_var_v", _tor_var_v);
    // IgaDebug::CheckMatrix(expected_data, "_cur_var_n_2", _curve_var_var_n);
    // IgaDebug::CheckMatrix(expected_data, "_cur_var_v_2", _curve_var_var_v);
    // IgaDebug::CheckMatrix(expected_data, "_tor_var_n_2", _tor_var_var_n);
    // IgaDebug::CheckMatrix(expected_data, "_tor_var_v_2", _tor_var_var_v);
  }




/**
 * Calculates the Cross Product between a Vector and a Matrix
 * @param[in]   vec     Input Vector [3,1]
 * @param[in]   mat     Input Matrix [3,3]
 * @return      vec_mat Output Matrix [3,3]
 */
BoundedMatrix<double,3,3> IgaBeamElement::CrossProductVectorMatrix(
    BoundedVector<double,3> vec,
    BoundedMatrix<double,3,3> mat)
{
    KRATOS_TRY;

    BoundedMatrix<double,3,3> vec_mat;
    vec_mat.clear();

    int permutation[3][3][3];
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){
                permutation[i][j][k] = 0; 
            }
        }
    }
    permutation[0][1][2] = 1;
    permutation[2][0][1] = 1;
    permutation[1][2][0] = 1;

    permutation[0][2][1] = -1;
    permutation[1][0][2] = -1;
    permutation[2][1][0] = -1; 

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){
                for (int l = 0; l < 3; l++){
                    vec_mat(i,j) += permutation[i][k][l] * vec(k) * mat(l,j); 
                }
            }
        }
    }
    return vec_mat;
    KRATOS_CATCH("")
}

/**
 * Calculates the Cross Product between a Vector and a Matrix
 * @param[in]   vec     Input Vector [n.a ,1]
 * @param[in]   mat     Input Matrix [n.a, n.a]
 * @return      vec_mat Output Matrix [n.a, n.a]
 */
Matrix IgaBeamElement::CrossProductVectorMatrix(
    Vector vec,
    Matrix mat)
{
    KRATOS_TRY;

    int size_vec = vec.size();
    int size_mat1 = mat.size1();
    int size_mat2 = mat.size2();
    
    Matrix vec_mat;
    vec_mat.resize(size_mat1, size_mat2);
    vec_mat.clear();

    if ((size_vec == size_mat1) && (size_vec == size_mat2))
    {
        int permutation[3][3][3];
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                for (int k = 0; k < 3; k++){
                    permutation[i][j][k] = 0; 
                }
            }
        }
        permutation[0][1][2] = 1;
        permutation[2][0][1] = 1;
        permutation[1][2][0] = 1;

        permutation[0][2][1] = -1;
        permutation[1][0][2] = -1;
        permutation[2][1][0] = -1; 

        Vector sort1;
        sort1.resize(3);
        sort1.clear();
        Vector sort2;
        sort2.resize(3);
        sort2.clear(); 

        int tmp; 
        
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                for (int k = 0; k < 3; k++){
                    for (int l = 0; l < 3; l++){
                        sort1[0] = j;
                        sort1[1] = k;
                        sort1[2] = l; 
                        
                        for (size_t n = 3; n > 1; n--){
                            for (size_t m = 0; i < n-1; m++){
                                if (sort1[i] > sort1[i+1])
                                {
                                    tmp = sort1[i];
                                    sort1[i] = sort1[i+1];
                                    sort1[i+1] = tmp;
                                }
                            }
                        }
                        for ( size_t o = 0; o < 3; o++)
                        {
                            if (j == sort1[o]) sort2[0] = o;
                            if (k == sort1[o]) sort2[1] = o;
                            if (l == sort1[o]) sort2[2] = o;
                        }
                        vec_mat(i, j) += permutation[i][k][l] * vec(k) * mat(l, j); 
                    }
                }
            }
        }
    }
    else 
    {
        KRATOS_ERROR << "Sizes of vector and matrix don't match!" << std::endl; 
    }

    return vec_mat;
    KRATOS_CATCH("")
}

/**
 * Calculates the Cross Product between a Matrix and a Vector
 * @param[in]   vec     Input Vector [3,1]
 * @param[in]   mat     Input Matrix [3,3]
 * @return      vec_mat Output Matrix [3,3]
 */
BoundedMatrix<double,3,3> IgaBeamElement::CrossProductMatrixVector(
    BoundedMatrix<double,3,3> mat,
    BoundedVector<double,3> vec)
{
    KRATOS_TRY;

    BoundedMatrix<double,3,3> vec_mat;
    vec_mat.clear();

    int permutation[3][3][3];
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){
                permutation[i][j][k] = 0; 
            }
        }
    }    
    permutation[0][1][2] = 1;
    permutation[2][0][1] = 1;
    permutation[1][2][0] = 1;

    permutation[0][2][1] = -1;
    permutation[1][0][2] = -1;
    permutation[2][1][0] = -1; 

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){
                for (int l = 0; l < 3; l++){
                    vec_mat(i,j) += permutation[i][k][l] * mat(i, k) * vec(l); 
                }
            }
        }
    }
    return vec_mat;
    KRATOS_CATCH("")
}





void IgaBeamElement::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "\"IgaBeamElement\" #" << Id();
}



void IgaBeamElement::comp_T_var(
    const Vector3& r1,
    std::vector<Vector3>& t_var)
{
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    double r1_dL = norm_2(r1);

    for (std::size_t r = 0; r != NumberOfDofs(); ++r) { 
        std::size_t xyz_r = GetDofTypeIndex(r);

        if (xyz_r != 3) {
            std::size_t i = GetShapeIndex(r);

            Vector3 r1_var;
            r1_var.clear();

            r1_var[xyz_r] = shape_derivatives(0, i);

            double r1_r1_var = inner_prod(r1, r1_var);
            
            t_var[r] = r1_var / r1_dL - r1 * r1_r1_var / std::pow(r1_dL, 3);
        } else {
            t_var[r].clear();
        }
    }
}

void IgaBeamElement::comp_T_var_var(
    const Vector3& r1,
    std::vector<Vector3>& t_var_var)
{
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    double r1_dL = norm_2(r1);
    double r1_pow3 = pow(r1_dL, 3);
    double r1_pow5 = pow(r1_dL, 5);
    
    std::vector<Vector3> r1_var(NumberOfDofs());

    for (std::size_t i = 0; i != NumberOfNodes(); ++i) {
        std::size_t r = i * DofsPerNode();
        
        r1_var[r + 0].clear();
        r1_var[r + 1].clear();
        r1_var[r + 2].clear();
        r1_var[r + 3].clear();

        r1_var[r + 0](0) = shape_derivatives(0, i);
        r1_var[r + 1](1) = shape_derivatives(0, i);
        r1_var[r + 2](2) = shape_derivatives(0, i);
    }

    Vector r1_r1_var(NumberOfDofs());

    for (std::size_t r = 0; r != NumberOfDofs(); ++r) { 
        std::size_t xyz_r = GetDofTypeIndex(r);

        if (xyz_r == 3) {
            r1_r1_var[r] = 0;
        } else {
            r1_r1_var[r] = inner_prod(r1_var[r], r1);
        }
    }

    for (std::size_t r = 0; r != NumberOfDofs(); ++r) { 
        std::size_t xyz_r = GetDofTypeIndex(r);

        for (std::size_t s = 0; s != NumberOfDofs(); ++s) { 
            std::size_t xyz_s = GetDofTypeIndex(s);

            if (xyz_r != 3 && xyz_s != 3) {
                double r1_var_r1_var = inner_prod(r1_var[r], r1_var[s]);

                t_var_var[r * NumberOfDofs() + s] = 3 * ((r1_r1_var[r] * r1_r1_var[s]) * r1) / r1_pow5 + (-(r1_var[r] * r1_r1_var[s]) - (r1_var[s] * r1_r1_var[r])) / r1_pow3 - (r1_var_r1_var * r1) / r1_pow3;
            } else {
                t_var_var[r * NumberOfDofs() + s].clear();
            }
        }
    }
}

void IgaBeamElement::comp_T_der_var(
    const Vector3& r1,
    const Vector3& r2,
    std::vector<Vector3>& t_der_var)
{
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    double r1r11 = inner_prod(r1, r2);
    double r11r11 = inner_prod(r2, r2);
    double r1_dL = norm_2(r1);
    
    std::vector<Vector3> r1_var(NumberOfDofs());
    
    for (size_t i = 0; i != NumberOfNodes(); ++i) {
        size_t r = i * DofsPerNode();
        
        r1_var[r + 0].clear();
        r1_var[r + 1].clear();
        r1_var[r + 2].clear();
        r1_var[r + 3].clear();

        r1_var[r + 0](0) = shape_derivatives(0, i);
        r1_var[r + 1](1) = shape_derivatives(0, i);
        r1_var[r + 2](2) = shape_derivatives(0, i);
    }
    
    std::vector<Vector3> r11_var(NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) { 
        for (size_t t = 0; t != 3; ++t) {
            size_t xyz_r = GetDofTypeIndex(r);
            size_t i = GetShapeIndex(r);
            if (t == xyz_r) {
                r11_var[r][t] = shape_derivatives(1, i);
            } else {
                r11_var[r][t] = 0;
            }
        }
    }

    Vector r1_r1_var(NumberOfDofs());
    Vector r11_r1_var(NumberOfDofs());
    Vector r1_r11_var(NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);

        if (xyz_r > 2) {
            r1_r1_var[r] = 0;
            r11_r1_var[r] = 0;
            r1_r11_var[r] = 0;
        } else {
            r1_r1_var[r] = inner_prod(r1_var[r], r1);
            r11_r1_var[r] = inner_prod(r1_var[r], r2);
            r1_r11_var[r] = inner_prod(r11_var[r], r1);
        }
    }
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);

        if (xyz_r > 2) {
            t_der_var[r].clear();
        } else {
            t_der_var[r] = r11_var[r] / r1_dL - r2 * r1_r1_var[r] / pow(r1_dL, 3) + 3 * r1_r1_var[r] * r1r11 * r1 / pow(r1_dL, 5) - 1.0 / pow(r1_dL, 3) * (r1_var[r] * r1r11 + (r1_r11_var[r] + r11_r1_var[r]) * r1);
        }
    }
}

void IgaBeamElement::comp_T_der_var_var(
    const Vector3& r1,
    const Vector3& r2,
    std::vector<Vector3>& t_der_var_var)
{
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    double r1r11 = inner_prod(r1, r2);
    double r11r11 = inner_prod(r2, r2);
    double r1_dL = norm_2(r1);
    
    double r1_pow3 = pow(r1_dL, 3);
    double r1_pow5 = pow(r1_dL, 5);
    double r1_pow7 = pow(r1_dL, 7);

    std::vector<Vector3> r1_var(NumberOfDofs());
    std::vector<Vector3> r11_var(NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) { 
        for (size_t t = 0; t != 3; ++t) {
            size_t xyz = GetDofTypeIndex(r);
            size_t i = GetShapeIndex(r);
            if (t == xyz) {
                r1_var[r][t] = shape_derivatives(0, i);
                r11_var[r][t] = shape_derivatives(1, i);
            } else {
                r1_var[r][t] = 0;
                r11_var[r][t] = 0;
            }
        }
    }

    Vector r1_r1_var(NumberOfDofs());
    Vector r11_r1_var(NumberOfDofs());
    Vector r1_r11_var(NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) { 
        size_t xyz = GetDofTypeIndex(r);

        if (xyz > 2) {
            r1_r1_var[r] = 0;
        } else {
            r1_r1_var[r] = inner_prod(r1_var[r], r1);
            r11_r1_var[r] = inner_prod(r2, r1_var[r]);
            r1_r11_var[r] = inner_prod(r1, r11_var[r]);
        }
    }
    
    Vector r1_var_r1_var(NumberOfDofs() * NumberOfDofs());
    Vector r1_var_r11_var(NumberOfDofs() * NumberOfDofs());
    Vector r11_var_r1_var(NumberOfDofs() * NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); ++r) { 
        for (size_t s = 0; s != NumberOfDofs(); ++s) { 
            r1_var_r1_var[r * NumberOfDofs() + s] = inner_prod(r1_var[r], r1_var[s]);
            r1_var_r11_var[r * NumberOfDofs() + s] = inner_prod(r1_var[r], r11_var[s]);
            r11_var_r1_var[r * NumberOfDofs() + s] = inner_prod(r11_var[r], r1_var[s]);
        }
    }
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyzr = GetDofTypeIndex(r);

        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyzs = GetDofTypeIndex(s);

            if (xyzr != 3 && xyzs != 3) {
                t_der_var_var[r * NumberOfDofs() + s] = (-(r11_var[r] * r1_r1_var[s]) - (r11_var[s] * r1_r1_var[r]) - r2 * r1_var_r1_var[r * NumberOfDofs() + s]) / r1_pow3;
                t_der_var_var[r * NumberOfDofs() + s] += 3 * ((r1_r1_var[r] * r2 * r1_r1_var[s])) / r1_pow5;
                t_der_var_var[r * NumberOfDofs() + s] += 3 * ((r1_var_r1_var[r * NumberOfDofs() + s] * r1r11 * r1)) / r1_pow5;
                t_der_var_var[r * NumberOfDofs() + s] += 3 * ((r1_r1_var[r] * (r11_r1_var[s] + r1_r11_var[s]) * r1)) / r1_pow5;
                t_der_var_var[r * NumberOfDofs() + s] += 3 * ((r1_r1_var[r] * r1_var[s]) * r1r11) / r1_pow5;
                t_der_var_var[r * NumberOfDofs() + s] += 3 * ((r1_r1_var[s] * r1_var[r]) * r1r11) / r1_pow5;
                t_der_var_var[r * NumberOfDofs() + s] += -15 * (r1_r1_var[r] * r1_r1_var[s]) * r1r11 * r1 / r1_pow7;
                t_der_var_var[r * NumberOfDofs() + s] += (-(r1_var_r11_var[r * NumberOfDofs() + s] + r11_var_r1_var[r * NumberOfDofs() + s]) * r1 - ((r1_r11_var[r] + r11_r1_var[r]) * r1_var[s]) - ((r1_r11_var[s] + r11_r1_var[s]) * r1_var[r])) / r1_pow3;
                t_der_var_var[r * NumberOfDofs() + s] += 3 * ((r1_r1_var[s] * (r11_r1_var[r] + r1_r11_var[r]) * r1)) / r1_pow5;
            } else {
                t_der_var_var[r * NumberOfDofs() + s].clear();
            }
        }
    }
}

BoundedMatrix<double, 3, 3> IgaBeamElement::cross_v_identity(
    const Vector3& v)
{
    BoundedMatrix<double, 3, 3> result;

    result(0, 0) = 0.0;
    result(0, 1) = -v(2);
    result(0, 2) = v(1);
    result(1, 0) = v(2);
    result(1, 1) = 0.0;
    result(1, 2) = -v(0);
    result(2, 0) = -v(1);
    result(2, 1) = v(0);
    result(2, 2) = 0.0;

    return result;
}

void IgaBeamElement::comp_rodrigues(
    const Vector3& v,
    const double& phi,
    BoundedMatrix<double, 3, 3>& rodrigues)
{
    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);
    
    rodrigues = cos_phi * IdentityMatrix(3) + cross_v_identity(sin_phi * v);
}

// check!
void IgaBeamElement::comp_rodrigues_der(
    const Vector3& v,
    const Vector3& v_der,
    const double& phi,
    const double& phi_der,
    BoundedMatrix<double, 3, 3>& rodrigues_der)
{
    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);
    
    auto c_v = cross_v_identity(v);
    auto c_d = cross_v_identity(v_der);

    rodrigues_der = cos_phi * phi_der * c_v + sin_phi * (c_d - phi_der * IdentityMatrix(3));
}

// check!
void IgaBeamElement::comp_rodrigues_der_var(
    const Vector3& v,
    const std::vector<Vector3>& v_var,
    const Vector3& v_der,
    const std::vector<Vector3>& v_der_var,
    const double& phi,
    const double& phi_der,
    std::vector<BoundedMatrix<double, 3, 3>>& rodrigues_der_var)
{
    Vector& shape = GetValue(SHAPE_FUNCTION_VALUES);
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);
    
    auto c_v = cross_v_identity(v);
    auto c_d = cross_v_identity(v_der);

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t i = GetShapeIndex(r);

        double phi_r = shape[i];
        double phi_der_r = shape_derivatives(0, i);
        BoundedMatrix<double, 3, 3> c_vr = cross_v_identity(v_var[r]);
        BoundedMatrix<double, 3, 3> c_dr = cross_v_identity(v_der_var[r]);

        if (GetDofTypeIndex(r) == 3) {
            rodrigues_der_var[r] = (-phi_der_r * sin_phi - phi_der * cos_phi * phi_r) * IdentityMatrix(3) + (phi_der_r * cos_phi - phi_der * sin_phi * phi_r) * c_v + cos_phi * phi_r * c_d;
        } else {
            rodrigues_der_var[r] = cos_phi * phi_der * c_vr + sin_phi * c_dr;
        }
    }      
}

// check!
void IgaBeamElement::comp_rodrigues_der_var_var(
    const Vector3& v, 
    const std::vector<Vector3>& v_var, 
    const std::vector<Vector3>& v_var_var, 
    const Vector3& v_der, 
    const std::vector<Vector3>& v_der_var, 
    const std::vector<Vector3>& v_der_var_var, 
    const double& phi, 
    const double& phi_der, 
    std::vector<BoundedMatrix<double, 3, 3>>& rodrigues_der_var_var)
{
    Vector& shape = GetValue(SHAPE_FUNCTION_VALUES);
    Matrix& shape_derivatives = GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);
    
    auto c_v = cross_v_identity(v);
    auto c_d = cross_v_identity(v_der);
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t i = GetShapeIndex(r);

        double phi_r = GetDofTypeIndex(r) == 3 ? shape[i] : 0.0;
        double phi_der_r = GetDofTypeIndex(r) == 3 ? shape_derivatives(0, i) : 0.0;

        BoundedMatrix<double, 3, 3> c_vr = cross_v_identity(v_var[r]);
        BoundedMatrix<double, 3, 3> c_dr = cross_v_identity(v_der_var[r]);

        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t j = GetShapeIndex(s);

            size_t rs = r * NumberOfDofs() + s;

            double phi_s = GetDofTypeIndex(s) == 3 ? shape[j] : 0.0;
            double phi_der_s = GetDofTypeIndex(s) == 3 ? shape_derivatives(0, j) : 0.0;
            
            BoundedMatrix<double, 3, 3> c_vs = cross_v_identity(v_var[s]);
            BoundedMatrix<double, 3, 3> c_ds = cross_v_identity(v_der_var[s]);
            
            BoundedMatrix<double, 3, 3> c_vrs = cross_v_identity(v_var_var[rs]);
            BoundedMatrix<double, 3, 3> c_drs = cross_v_identity(v_der_var_var[rs]);

            rodrigues_der_var_var[rs] = -(phi_der_r * phi_s * cos_phi + phi_der_s * phi_r * cos_phi - sin_phi * phi_der * phi_r * phi_s) * IdentityMatrix(3) -(phi_der_r * phi_s + phi_der_s * phi_r) * sin_phi * c_v + cos_phi * (phi_der_r * c_vs + phi_der_s * c_vr - phi_der * phi_r * phi_s * c_v + phi_der * c_vrs + phi_r * c_ds + phi_s * c_dr) - sin_phi * (phi_der * (phi_r * c_vs + phi_s * c_vr) + phi_r * phi_s * c_d) + sin_phi * c_drs;
        }
    }
}

// check!
void IgaBeamElement::comp_rodrigues_var(
    const Vector3& v,
    const std::vector<Vector3>& v_var, 
    const double& phi, 
    std::vector<BoundedMatrix<double, 3, 3>>& rodrigues_var)
{
    Vector& shape = GetValue(SHAPE_FUNCTION_VALUES);

    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);    

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        rodrigues_var[r] = sin_phi * cross_v_identity(v_var[r]);

        if (GetDofTypeIndex(r) == 3) {
            size_t i = GetShapeIndex(r);
            
            rodrigues_var[r] += cos_phi * shape[i] * cross_v_identity(v) - sin_phi * shape[i] * IdentityMatrix(3);
        }
    }
}

// check!
void IgaBeamElement::comp_rodrigues_var_var(
    const Vector3& v, 
    const std::vector<Vector3>& v_var, 
    const std::vector<Vector3>& v_var_var, 
    const double& phi, 
    std::vector<BoundedMatrix<double, 3, 3>>& rodrigues_var_var)
{
    Vector& shape = GetValue(SHAPE_FUNCTION_VALUES);

    BoundedMatrix<double, 3, 3> c_v = cross_v_identity(v);

    double sin_phi = std::sin(phi);
    double cos_phi = std::cos(phi);

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t i = GetShapeIndex(r);

        double phi_r = GetDofTypeIndex(r) == 3 ? shape[i] : 0.0;
        BoundedMatrix<double, 3, 3> c_vr = cross_v_identity(v_var[r]);

        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t j = GetShapeIndex(s);
            
            size_t rs = r * NumberOfDofs() + s;

            double phi_s = GetDofTypeIndex(s) == 3 ? shape[j] : 0.0;

            BoundedMatrix<double, 3, 3> c_vs = cross_v_identity(v_var[s]);

            BoundedMatrix<double, 3, 3> c_vrs = cross_v_identity(v_var_var[rs]);

            rodrigues_var_var[rs] = -cos_phi * phi_r * phi_s * IdentityMatrix(3);
            
            if (GetDofTypeIndex(r) == 3 || GetDofTypeIndex(s) == 3) {
                rodrigues_var_var[rs] += cos_phi * (phi_r * c_vs + phi_s * c_vr) - sin_phi * phi_r * phi_s * c_v;
            } else {
                rodrigues_var_var[rs] += sin_phi * c_vrs;
            }
        }
    }
}

// check!
void IgaBeamElement::comp_lambda(
    const Vector3& v1,
    const Vector3& v2,
    BoundedMatrix<double, 3, 3>& lambda)
{
    Vector3 v1_x_v2 = Cross(v1, v2);
    double v1_d_v2 = inner_prod(v1, v2);

    lambda = v1_d_v2 * IdentityMatrix(3) + cross_v_identity(v1_x_v2);

    if (v1_d_v2 + 1.0 > 1e-7) {
        lambda += outer_prod(v1_x_v2, v1_x_v2) * 1.0 / (1.0 + v1_d_v2);
    } else {
        double l_v1_x_v2 = norm_2(v1_x_v2);

        Vector3 e_hat = v1_x_v2;

        if (l_v1_x_v2 > 1e-7) {
            e_hat /= l_v1_x_v2;
            lambda += outer_prod(e_hat, e_hat) * (1 - v1_d_v2);
        }
    }
}

// check!
void IgaBeamElement::comp_lambda_var(
    const Vector3& v1,
    const Vector3& v2,
    const std::vector<Vector3>& v2_var,
    std::vector<BoundedMatrix<double, 3, 3>>& lambda_var)
{
    using std::pow;

    Vector3 v1_x_v2 = Cross(v1, v2);
    double T0_T = inner_prod(v1, v2);
    double d = 1.0 / (1.0 + T0_T);

    BoundedMatrix<double, 3, 3> c_1 = cross_v_identity(v1);

    for (size_t r = 0; r != NumberOfDofs(); ++r)
    {            
        if (GetDofTypeIndex(r) != 3) {
            Vector3 v1_x_v2_var = Cross(v1, v2_var[r]);
            double t0_t_var = inner_prod(v2_var[r], v1);

            BoundedMatrix<double, 3, 3> o = outer_prod(v1_x_v2_var, v1_x_v2);

            lambda_var[r] = t0_t_var * IdentityMatrix(3) + cross_v_identity(v1_x_v2_var) - t0_t_var * pow(d, 2) * outer_prod(v1_x_v2, v1_x_v2) + d * (o + trans(o));
        } else {
            lambda_var[r].clear();
        }
    }
}

void IgaBeamElement::comp_lambda_var_var(
    const Vector3& v1,
    const Vector3& v2,
    const std::vector<Vector3>& v2_var,
    const std::vector<Vector3>& v2_var_var,
    std::vector<BoundedMatrix<double, 3, 3>>& lambda_var_var)
{
    Vector3 cross_vec1_vec2 = Cross(v1, v2);
 
    Vector T0_T_var(NumberOfDofs());
 
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz = GetDofTypeIndex(r);
 
        if (xyz > 2) {
            T0_T_var[r] = 0;
        } else {
            T0_T_var[r] = inner_prod(v2_var[r], v1);
        }
    }
 
    double T0_T = inner_prod(v1, v2);
 
    Matrix T0_T_var_var(NumberOfDofs(), NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyzr = GetDofTypeIndex(r);
 
        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyzs = GetDofTypeIndex(s);
 
            if (xyzr > 2 || xyzs > 2) {
                T0_T_var_var(r, s) = 0;
            } else {
                T0_T_var_var(r, s) = inner_prod(v2_var_var[r * NumberOfDofs() + s], v1);
            }
        }
    }
    
    std::vector<Vector3> cross_vec1_vec2_var(NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz = GetDofTypeIndex(r);
 
        if (xyz > 2) {
            cross_vec1_vec2_var[r].clear();
        } else {
            cross_vec1_vec2_var[r] = prod(cross_v_identity(v1), v2_var[r]);
        }
    }
    
    std::vector<Vector3> cross_vec1_vec2_var_var(NumberOfDofs() * NumberOfDofs());
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);
 
        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyz_s = GetDofTypeIndex(s);
        
            if (xyz_r > 2 || xyz_s > 2) {
                cross_vec1_vec2_var_var[r * NumberOfDofs() + s].clear();
            } else {
                cross_vec1_vec2_var_var[r * NumberOfDofs() + s] = prod(cross_v_identity(v1), v2_var_var[r * NumberOfDofs() + s]);
            }
        }
    }
 
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);
 
        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyz_s = GetDofTypeIndex(s);
            
            if (xyz_r > 2 || xyz_s > 2) {
                lambda_var_var[r * NumberOfDofs() + s].clear();
            } else {
                lambda_var_var[r * NumberOfDofs() + s] = T0_T_var_var(r, s) * IdentityMatrix(3);
                lambda_var_var[r * NumberOfDofs() + s] += cross_v_identity(cross_vec1_vec2_var_var[r * NumberOfDofs() + s]);
                lambda_var_var[r * NumberOfDofs() + s] += (2 * T0_T_var[r] * T0_T_var[s] / pow(1.0 + T0_T, 3) - T0_T_var_var(r, s) / pow(1.0 + T0_T, 2)) *  outer_prod(cross_vec1_vec2, cross_vec1_vec2);
                lambda_var_var[r * NumberOfDofs() + s] += -T0_T_var[r] / pow(1.0 + T0_T, 2) * (outer_prod(cross_vec1_vec2_var[s], cross_vec1_vec2) + outer_prod(cross_vec1_vec2, cross_vec1_vec2_var[s])) - T0_T_var[s] / pow(1.0 + T0_T, 2) * (outer_prod(cross_vec1_vec2_var[r], cross_vec1_vec2) + outer_prod(cross_vec1_vec2, cross_vec1_vec2_var[r]));
                lambda_var_var[r * NumberOfDofs() + s] += 1.0 / (1.0 + T0_T) * (outer_prod(cross_vec1_vec2_var_var[r * NumberOfDofs() + s], cross_vec1_vec2) + outer_prod(cross_vec1_vec2_var[r], cross_vec1_vec2_var[s]) + outer_prod(cross_vec1_vec2_var[s], cross_vec1_vec2_var[r]) + outer_prod(cross_vec1_vec2, cross_vec1_vec2_var_var[s * NumberOfDofs() + r]));
            }
        }
    }
}


// check!
void IgaBeamElement::comp_lambda_der(
    const Vector3& v1,
    const Vector3& v1_der,
    const Vector3& v2,
    const Vector3& v2_der,
    BoundedMatrix<double, 3, 3>& lambda_der)
{
    using std::pow;

    double T0_T  = inner_prod(v1, v2);
    double T0_T_1 = inner_prod(v1, v2_der) + inner_prod(v1_der, v2);
    Vector3 T0xT = Cross(v1, v2);
    Vector3 T0xT_1 = Cross(v1, v2_der) + Cross(v1_der, v2);

    double d = 1.0 / (1.0 + T0_T);

    BoundedMatrix<double, 3, 3> o = outer_prod(T0xT_1, T0xT) + outer_prod(T0xT, T0xT_1);

    lambda_der = T0_T_1 * IdentityMatrix(3) + cross_v_identity(T0xT_1) - T0_T_1 * pow(d, 2) * outer_prod(T0xT, T0xT) + d * o;
}

// check!
void IgaBeamElement::comp_lambda_der_var(
    const Vector3& v1,
    const Vector3& v1_der,
    const Vector3& v2,
    const std::vector<Vector3>& v2_var,
    const Vector3& v2_der,
    const std::vector<Vector3>& v2_der_var,
    std::vector<BoundedMatrix<double, 3, 3>>& lambda_der_var)
{
    double T0_T  = inner_prod(v1, v2);
    double T0_T1 = inner_prod(v1, v2_der);
    double T01_T = inner_prod(v1_der, v2);
    
    Vector3 cross_vec1_vec2 = Cross(v1, v2);
    Vector3 cross_vec1_vec2_der = Cross(v1, v2_der);
    Vector3 cross_vec1_der_vec2 = Cross(v1_der, v2);
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz = GetDofTypeIndex(r);
    
        if (xyz > 2) {
            lambda_der_var[r].clear();
        } else {
            Vector3 cross_vec1_vec2_var = prod(cross_v_identity(v1), v2_var[r]);
            Vector3 cross_vec1_vec2_der_var = prod(cross_v_identity(v1), v2_der_var[r]);
            Vector3 cross_vec1_der_vec2_var = prod(cross_v_identity(v1_der), v2_var[r]);
            
            double T0_T_var = inner_prod(v2_var[r], v1);
            double T0_T_der_var = inner_prod(v2_der_var[r], v1);
            double T0_der_T_var = inner_prod(v2_var[r], v1_der);
            
            lambda_der_var[r] = (T0_T_der_var + T0_der_T_var) * IdentityMatrix(3);
            lambda_der_var[r] += cross_v_identity(cross_vec1_vec2_der_var) + cross_v_identity(cross_vec1_der_vec2_var);
            lambda_der_var[r] += (2 * (T0_T_var) * (T0_T1 + T01_T) / pow(1.0 + T0_T, 3) - (T0_T_der_var + T0_der_T_var) / pow(1.0 + T0_T, 2)) * outer_prod(cross_vec1_vec2, cross_vec1_vec2);
            lambda_der_var[r] += -(T0_T1 + T01_T) / pow(1.0 + T0_T, 2) * (outer_prod(cross_vec1_vec2_var, cross_vec1_vec2) + outer_prod(cross_vec1_vec2, cross_vec1_vec2_var));
            lambda_der_var[r] += -(T0_T_var) / pow(1.0 + T0_T, 2) * (outer_prod(cross_vec1_vec2_der + cross_vec1_der_vec2, cross_vec1_vec2) + outer_prod(cross_vec1_vec2, cross_vec1_vec2_der + cross_vec1_der_vec2));
            lambda_der_var[r] += 1.0 / (1.0 + T0_T) * (outer_prod(cross_vec1_vec2_der_var + cross_vec1_der_vec2_var, cross_vec1_vec2) + outer_prod(cross_vec1_vec2_var, cross_vec1_vec2_der + cross_vec1_der_vec2) + outer_prod(cross_vec1_vec2_der + cross_vec1_der_vec2, cross_vec1_vec2_var) + outer_prod(cross_vec1_vec2, cross_vec1_vec2_der_var + cross_vec1_der_vec2_var));
        }
    }
}

// check!
void IgaBeamElement::comp_lambda_der_var_var(
    const Vector3& v1,
    const Vector3& v1_der,
    const Vector3& v2,
    const std::vector<Vector3>& v2_var,
    const std::vector<Vector3>& v2_var_var,
    const Vector3& v2_der,
    const std::vector<Vector3>& v2_der_var,
    const std::vector<Vector3>& v2_der_var_var,
    std::vector<BoundedMatrix<double, 3, 3>>& lambda_der_var_var)
{
    double T0_T = inner_prod(v1,v2);
    double T0_T1 = inner_prod(v1, v2_der);
    double T01_T = inner_prod(v1_der, v2);
    double T0_T_1 = T0_T1 + T01_T;
    
    std::vector<Vector3> T0xvec2_var(NumberOfDofs());
    std::vector<Vector3> T0xvec2_der_var(NumberOfDofs());
    std::vector<Vector3> T0_derxvec2_var(NumberOfDofs());

    Vector3 T0xvec2 = Cross(v1, v2);
    Vector3 T0xvec2_der = Cross(v1, v2_der);
    Vector3 T0_derxvec2 = Cross(v1_der, v2);

    Vector T0_T_var(NumberOfDofs());
    Vector T0_der_T_var(NumberOfDofs());
    Vector T0_T_der_var(NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz = GetDofTypeIndex(r);

        if (xyz > 2) {
            T0_T_var[r] = 0;
            T0_der_T_var[r] = 0; // das hat...
            T0_T_der_var[r] = 0; // ...gefehlt
        } else {
            T0_T_var[r] = inner_prod(v2_var[r], v1);
            T0_der_T_var[r] = inner_prod(v2_var[r], v1_der);
            T0_T_der_var[r] = inner_prod(v2_der_var[r], v1);
        }
    }
    
    Matrix T0_T_var_var(NumberOfDofs(), NumberOfDofs());
    Matrix T0_T_der_var_var(NumberOfDofs(), NumberOfDofs());
    Matrix T0_der_T_var_var(NumberOfDofs(), NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyzr = GetDofTypeIndex(r);

        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyzs = GetDofTypeIndex(s);

            if (xyzr > 2 || xyzs > 2) {
                T0_T_var_var(r, s) = 0;
                T0_T_der_var_var(r, s) = 0;
                T0_der_T_var_var(r, s) = 0;
            } else {
                T0_T_var_var(r, s) = inner_prod(v2_var_var[r * NumberOfDofs() + s], v1);
                T0_T_der_var_var(r, s) = inner_prod(v2_der_var_var[r * NumberOfDofs() + s], v1);
                T0_der_T_var_var(r, s) = inner_prod(v2_var_var[r * NumberOfDofs() + s], v1_der);
            }
        }
    }
    
    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);

        if (xyz_r > 2) {
            T0xvec2_var[r].clear();
        } else {
            T0xvec2_var[r] = prod(cross_v_identity(v1), v2_var[r]);
            T0xvec2_der_var[r] = prod(cross_v_identity(v1), v2_der_var[r]);
            T0_derxvec2_var[r] = prod(cross_v_identity(v1_der), v2_var[r]);
        }
    }
    
    std::vector<Vector3> T0xvec2_var_var(NumberOfDofs() * NumberOfDofs());
    std::vector<Vector3> T0xvec2_der_var_var(NumberOfDofs() * NumberOfDofs());
    std::vector<Vector3> T0_derxvec2_var_var(NumberOfDofs() * NumberOfDofs());

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);

        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyz_s = GetDofTypeIndex(s);

            if (xyz_r < 3 || xyz_s < 3) {
                T0xvec2_var_var[r * NumberOfDofs() + s] = prod(cross_v_identity(v1), v2_var_var[r * NumberOfDofs() + s]);
                T0xvec2_der_var_var[r * NumberOfDofs() + s] = prod(cross_v_identity(v1), v2_der_var_var[r * NumberOfDofs() + s]);
                T0_derxvec2_var_var[r * NumberOfDofs() + s] = prod(cross_v_identity(v1_der), v2_var_var[r * NumberOfDofs() + s]);
            } else {
                T0xvec2_var_var[r * NumberOfDofs() + s].clear();
                T0xvec2_der_var_var[r * NumberOfDofs() + s].clear();
                T0_derxvec2_var_var[r * NumberOfDofs() + s].clear();
            }
        }
    }

    for (size_t r = 0; r != NumberOfDofs(); ++r) {
        size_t xyz_r = GetDofTypeIndex(r);

        for (size_t s = 0; s != NumberOfDofs(); ++s) {
            size_t xyz_s = GetDofTypeIndex(s);

            lambda_der_var_var[r * NumberOfDofs() + s] = (T0_T_der_var_var(r, s) + T0_der_T_var_var(r, s)) * IdentityMatrix(3);
            lambda_der_var_var[r * NumberOfDofs() + s] += cross_v_identity(T0xvec2_der_var_var[r * NumberOfDofs() + s] + T0_derxvec2_var_var[r * NumberOfDofs() + s]);

            if (xyz_r != 3 && xyz_s != 3) {
                lambda_der_var_var[r * NumberOfDofs() + s] += (2 * (T0_T_var[r] * (T0_T_der_var[s] + T0_der_T_var[s]) + T0_T_1 * T0_T_var_var(r, s)) / pow(1.0 + T0_T, 3) - 6 * T0_T_1 * T0_T_var[r] * T0_T_var[s] / pow(1.0 + T0_T, 4) - (T0_T_der_var_var(r, s) + T0_der_T_var_var(r, s)) / pow(1.0 + T0_T, 2) + 2 * (T0_T_der_var[r] + T0_der_T_var[r]) * T0_T_var[s] / pow(1.0 + T0_T, 3)) * outer_prod(T0xvec2, T0xvec2);
                lambda_der_var_var[r * NumberOfDofs() + s] += (2 * T0_T_var[r] * T0_T_1 / pow(1.0 + T0_T, 3) - (T0_T_der_var[r] + T0_der_T_var[r]) / pow(1.0 + T0_T, 2)) * (outer_prod(T0xvec2_var[s], T0xvec2) + outer_prod(T0xvec2, T0xvec2_var[s])) + (2 * T0_T_var[s] * T0_T_1 / pow(1.0 + T0_T, 3) - (T0_T_der_var[s] + T0_der_T_var[s]) / pow(1.0 + T0_T, 2)) * (outer_prod(T0xvec2_var[r], T0xvec2) + outer_prod(T0xvec2, T0xvec2_var[r]));
                lambda_der_var_var[r * NumberOfDofs() + s] += -T0_T_1 / pow(1.0 + T0_T, 2) * (outer_prod(T0xvec2_var_var[r * NumberOfDofs() + s], T0xvec2) + outer_prod(T0xvec2_var[r], T0xvec2_var[s]) + outer_prod(T0xvec2_var[s], T0xvec2_var[r]) + outer_prod(T0xvec2, T0xvec2_var_var[r * NumberOfDofs() + s]));
                lambda_der_var_var[r * NumberOfDofs() + s] += (-T0_T_var_var(r, s) / pow(1.0 + T0_T, 2) + 2 * T0_T_var[r] * T0_T_var[s] / pow(1.0 + T0_T, 3)) * (outer_prod(T0xvec2_der + T0_derxvec2, T0xvec2) + outer_prod(T0xvec2, T0xvec2_der + T0_derxvec2));
                lambda_der_var_var[r * NumberOfDofs() + s] += (-T0_T_var[r] / pow(1.0 + T0_T, 2)) * (outer_prod(T0xvec2_der_var[s] + T0_derxvec2_var[s], T0xvec2) + outer_prod(T0xvec2_var[s], T0xvec2_der + T0_derxvec2) + outer_prod(T0xvec2_der + T0_derxvec2, T0xvec2_var[s]) + outer_prod(T0xvec2, T0xvec2_der_var[s] + T0_derxvec2_var[s]));
                lambda_der_var_var[r * NumberOfDofs() + s] += (-T0_T_var[s] / pow(1.0 + T0_T, 2)) * (outer_prod(T0xvec2_der_var[r] + T0_derxvec2_var[r], T0xvec2) + outer_prod(T0xvec2_var[r], T0xvec2_der + T0_derxvec2) + outer_prod(T0xvec2_der + T0_derxvec2, T0xvec2_var[r]) + outer_prod(T0xvec2, T0xvec2_der_var[r] + T0_derxvec2_var[r]));
                lambda_der_var_var[r * NumberOfDofs() + s] += 1.0 / (1.0 + T0_T) * (outer_prod(T0xvec2_der_var_var[r * NumberOfDofs() + s] + T0_derxvec2_var_var[r * NumberOfDofs() + s], T0xvec2) + outer_prod(T0xvec2_var_var[r * NumberOfDofs() + s], T0xvec2_der + T0_derxvec2) + outer_prod(T0xvec2_der_var[s] + T0_derxvec2_var[s], T0xvec2_var[r]) + outer_prod(T0xvec2_var[s], T0xvec2_der_var[r] + T0_derxvec2_var[r]) + outer_prod(T0xvec2_der_var[r] + T0_derxvec2_var[r], T0xvec2_var[s]) + outer_prod(T0xvec2_var[r], T0xvec2_der_var[s] + T0_derxvec2_var[s]) + outer_prod(T0xvec2_der+ T0_derxvec2, T0xvec2_var_var[r * NumberOfDofs() + s]) + outer_prod(T0xvec2, T0xvec2_der_var_var[r * NumberOfDofs() + s] + T0_derxvec2_var_var[r * NumberOfDofs() + s]));
            }
        }
    }
}

} // namespace Kratos