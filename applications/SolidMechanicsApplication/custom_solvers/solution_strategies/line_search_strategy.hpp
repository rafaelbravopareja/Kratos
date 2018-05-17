//
//   Project Name:        KratosSolidMechanicsApplication $
//   Created by:          $Author:            JMCarbonell $
//   Last modified by:    $Co-Author:                     $
//   Date:                $Date:               March 2018 $
//   Revision:            $Revision:                  0.0 $
//
//


#if !defined(KRATOS_LINE_SEARCH_STRATEGY_H_INCLUDED)
#define KRATOS_LINE_SEARCH_STRATEGY_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_solvers/solution_strategies/newton_raphson_strategy.hpp"


namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{


template<class TSparseSpace,
         class TDenseSpace, // = DenseSpace<double>,
         class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
         >
class LineSearchSolutionStrategy
    : public NewtonRaphsonStrategy<TSparseSpace, TDenseSpace, TLinearSolver>
{
 public:
  ///@name Type Definitions
  ///@{

  // Counted pointer of ClassName
  KRATOS_CLASS_POINTER_DEFINITION(LineSearchSolutionStrategy);

  typedef NewtonRaphsonStrategy<TSparseSpace, TDenseSpace, TLinearSolver>       BaseType;

  typedef typename BaseType::LocalFlagType                                 LocalFlagType;
  
  typedef typename BaseType::SystemVectorType                           SystemVectorType;
  
  typedef ConvergenceCriteria<TSparseSpace, TDenseSpace>        ConvergenceCriterionType;

  typedef typename BaseType::BuilderAndSolverType                   BuilderAndSolverType;

  typedef typename BaseType::SchemeType                                       SchemeType;

  typedef TLinearSolver                                                 LinearSolverType;

  ///@}
  ///@name Life Cycle

  ///@{

   
  /// Constructor.
   
  LineSearchSolutionStrategy(ModelPart& rModelPart,
                     typename SchemeType::Pointer pScheme,
                     typename BuilderAndSolverType::Pointer pBuilderAndSolver,
                     typename ConvergenceCriterionType::Pointer pConvergenceCriterion,
                     Flags& rOptions,
                     unsigned int MaxIterations = 30,
                     unsigned int LineSearchType = 0)
      : NewtonRaphsonStrategy<TSparseSpace, TDenseSpace, TLinearSolver>(rModelPart, pScheme, pBuilderAndSolver, pConvergenceCriterion, rOptions, MaxIterations), mType(LineSearchType)
  {mAlpha=1.0;}

  LineSearchSolutionStrategy(ModelPart& rModelPart,
                     typename SchemeType::Pointer pScheme,
                     typename LinearSolverType::Pointer pLinearSolver,
                     typename ConvergenceCriterionType::Pointer pConvergenceCriterion,
                     Flags& rOptions,
                     unsigned int MaxIterations = 30,
                     unsigned int LineSearchType = 0)      
      : NewtonRaphsonStrategy<TSparseSpace, TDenseSpace, TLinearSolver>(rModelPart, pScheme, pLinearSolver, pConvergenceCriterion, rOptions, MaxIterations), mType(LineSearchType)
  {mAlpha=1.0;}

  /// Destructor
  ~LineSearchSolutionStrategy() override {}

  ///@}
  ///@name Operators
  ///@{
  ///@}
  ///@name Operations
  ///@{
  ///@}
  ///@name Access
  ///@{
  ///@}
  ///@name Inquiry
  ///@{
  ///@}
  ///@name Friends
  ///@{
  ///@}

 protected:
  ///@name Protected static Member Variables
  ///@{
  ///@}
  ///@name Protected member Variables
  ///@{
  ///@}
  ///@name Protected Operators
  ///@{
  ///@}
  ///@name Protected Operations
  ///@{


  // LINESEARCH 
  void Update() override        
  {
    KRATOS_TRY

    //select line-search type:
    switch (mType){
      case 0:
        UpdateA();
        break;
      case 1:
        UpdateB();
        break;
      case 2:
        UpdateC();
        break;
      case 3:
        UpdateD();
        break;
      case 4:
        UpdateE();
        break;
    }
        
    KRATOS_CATCH( "" )       
  }
  

  ///@}
  ///@name Protected  Access
  ///@{
  ///@}
  ///@name Protected Inquiry
  ///@{
  ///@}
  ///@name Protected LifeCycle
  ///@{
  ///@}

 private:
  ///@name Static Member Variables
  ///@{
  ///@}
  ///@name Member Variables
  ///@{

  unsigned int mType;

  double mAlpha;
  
  ///@}
  ///@name Private Operators
  ///@{
  ///@}
  ///@name Private Operations
  ///@{

  //**************************************************************************
  //**************************************************************************

  // LINESEARCH (Secant type)
  void UpdateA()         
  {
    KRATOS_TRY

    SystemVectorType Dx0((*this->mpb).size()); 
    TSparseSpace::Assign(Dx0,1.0,(*this->mpDx));

    //compute slopes:
    
    //s0  (alpha=0) 
    double s0 = TSparseSpace::Dot((*this->mpDx),(*this->mpb));
        
    //s1  (alpha=1)
    double s1 = 0;
    ComputeUpdatedSlope(s1,Dx0);

    double alpha = mAlpha;
    if( s0 * s1 < 0 ){

      double s2 = s1; //current slope
      if( fabs(s1)<fabs(s0) )
        s2 = s0;

      double s_p   = s0;
      double nabla = 0.0;
      double delta = 1.0;
      alpha = 1.0;
      
      unsigned int iterations = 0;
      unsigned int max_iterations = 10;
      
      while(fabs(s2/s_p)>0.3 && iterations<max_iterations && fabs(s0)>1e-7 && fabs(s1)>1e-7 && (s0*s1)<0) {

        alpha = 0.5*(nabla+delta);
       
        //compute:
        TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
        ComputeUpdatedSlope(s2,Dx0);

        if( s2*s1 < 0 ){
          nabla = alpha;
          s0 = s2;
        }
        else if( s2 * s0 < 0 ){
          delta = alpha;
          s1 = s2;
        }
        else{
          break;
        }
        
        iterations++;
      }

      if( alpha > 1 )
        alpha = 1;

      if( alpha <= 0 )
        alpha = 0.001;
      
      mAlpha = alpha;     
    }

    //perform final update
    TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
    BaseType::Update();

    //restore
    TSparseSpace::Assign((*this->mpDx),1.0, Dx0);
    
    KRATOS_CATCH( "" )
  }
  
  //**************************************************************************
  //**************************************************************************

  // LINESEARCH (Bonet and Wood)
  void UpdateB()         
  {
    KRATOS_TRY

    SystemVectorType Dx0((*this->mpb).size()); 
    TSparseSpace::Assign(Dx0,1.0,(*this->mpDx));

    //compute slopes:
    
    //s0  (alpha=0) 
    double s0 = TSparseSpace::Dot((*this->mpDx),(*this->mpb));
        
    //s1  (alpha=1)
    double s1 = 0;
    ComputeUpdatedSlope(s1,Dx0);

    double alpha = mAlpha;
    if( s0 * s1 < 0 ){

      double s2 = s1; //current slope

      double nabla = 0.0;
      alpha = 1.0;
      
      unsigned int iterations = 0;
      unsigned int max_iterations = 3;
      
      while(fabs(s2)>0.5*fabs(s0) && iterations<max_iterations && fabs(s0)>1e-7 && fabs(s2)>1e-7 && (s0*s2)<0) {

        
        nabla = s0/s2;
        
        if( nabla < 0 ){
          alpha = 0.5*(nabla+std::sqrt(nabla*(nabla-4.0)));
        }
        else if( nabla > 0 && nabla <= 2 ){
          alpha = 0.5*nabla;
        }
        else{
          break;
        }
        
        //compute:
        TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
        ComputeUpdatedSlope(s2,Dx0);

        iterations++;
      }

      if( alpha > 1 )
        alpha = 1;

      if( alpha <= 0 )
        alpha = 0.001;

      mAlpha = alpha;     
    }

    //perform final update
    TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
    BaseType::Update();

    //restore
    TSparseSpace::Assign((*this->mpDx),1.0, Dx0);
    
    KRATOS_CATCH( "" )
  }
    

  //**************************************************************************
  //**************************************************************************

  // LINESEARCH (Error-based type)
  void UpdateC()         
  {
    KRATOS_TRY

    SystemVectorType Dx0((*this->mpb).size()); 
    TSparseSpace::Assign(Dx0,1.0,(*this->mpDx));
    
    //compute slopes:
    
    //s0  (alpha=0) 
    double s0 = TSparseSpace::Dot((*this->mpDx),(*this->mpb));
        
    //s1  (alpha=1)
    double s1 = 0;
    ComputeUpdatedSlope(s1,Dx0);

    double alpha = mAlpha;
    if( s0 * s1 < 0 ){

      double s2 = s1; //current slope
  
      double nabla = s0/s1;
      alpha = 1.0;
      
      unsigned int iterations = 0;
      unsigned int max_iterations = 10;
      
      while(fabs(s2/s0)>0.5 && iterations<max_iterations && fabs(s2)>1e-4) {

        if( nabla < 0 ){
          alpha = nabla/(0.5*(nabla-sqrt(nabla*(nabla-4.0))));
          //alpha = 0.5*(nabla+std::sqrt(nabla*(nabla-4.0)));
          //alpha = 0.5*nabla+std::sqrt(nabla*nabla*0.25-nabla);
        }
        else if( nabla > 0 && nabla <= 2 ){
          alpha = nabla * 0.5;
        }
        else{
          break;
        }
        
        //compute:
        TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
        ComputeUpdatedSlope(s2,Dx0);

        nabla = s0/s2;
        
        iterations++;
      }

      if( alpha > 1 )
        alpha = 1;

      if( alpha <= 0 )
        alpha = 0.001;
      
      mAlpha = alpha;     
    }

    //perform final update
    TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
    BaseType::Update();

    //restore
    TSparseSpace::Assign((*this->mpDx),1.0, Dx0);
    
    KRATOS_CATCH( "" )
  }
  
  //**************************************************************************
  //**************************************************************************

  // LINESEARCH (Crisfield, Wriggers)
  void UpdateD()         
  {
    KRATOS_TRY

    SystemVectorType Dx0((*this->mpb).size()); 
    TSparseSpace::Assign(Dx0,1.0,(*this->mpDx));

    //compute slopes:
    
    //s0  (alpha=0) 
    double s0 = TSparseSpace::Dot((*this->mpDx),(*this->mpb));
        
    //s1  (alpha=1)
    double s1 = 0;
    ComputeUpdatedSlope(s1,Dx0);

    double alpha = mAlpha; //new alpha
    if( s0 * s1 < 0 ){

      double s_c = 0; //current slope
      double s_p = 0; //previous slope
      double alpha_c = 1.0; //current_alpha
      double alpha_p = 0.0; //previous_alpha
      
      //s_p  (alpha=alpha_p)
      TSparseSpace::Assign((*this->mpDx),alpha_p,Dx0);
      ComputeUpdatedSlope(s_p,Dx0);

      //s_c  (alpha=alpha_c)
      TSparseSpace::Assign((*this->mpDx),alpha_c,Dx0);
      ComputeUpdatedSlope(s_c,Dx0);
     
      unsigned int iterations = 0;
      unsigned int max_iterations = 3;
      
      while(fabs(s_c)>0.8*fabs(s0) && iterations<=max_iterations && fabs(s_c)>1e-5 && fabs(s_p)>1e-5 && (s0*s1)<0) {

        alpha = alpha_c - s_c * (alpha_c-alpha_p) / (s_c-s_p);
        
        //update:             
        alpha_p = alpha_c;
        alpha_c = alpha;
        s_p = s_c;

        //compute:
        TSparseSpace::Assign((*this->mpDx),alpha_c,Dx0);
        ComputeUpdatedSlope(s_c,Dx0);

        iterations++;
      }

      if( alpha > 1 )
        alpha = 1;

      if( alpha <= 0 )
        alpha = 0.001;
      
      mAlpha = alpha;     
    }

    //perform final update
    TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
    BaseType::Update();

    //restore
    TSparseSpace::Assign((*this->mpDx),1.0, Dx0);
    
    KRATOS_CATCH( "" )
  }

  //**************************************************************************
  //**************************************************************************
  
  // LINESEARCH (Kratos core)
  void UpdateE()         
  {
    KRATOS_TRY

    SystemVectorType Dx0((*this->mpb).size());
    TSparseSpace::Assign(Dx0,1.0,(*this->mpDx));

    //compute residual without update
    double ro = 0;
    ComputeUpdatedResidualNorm(ro);

    //compute half step residual
    TSparseSpace::Assign((*this->mpDx),0.5,Dx0);
    double rh = 0;
    ComputeUpdatedResidualNorm(rh);

    //compute full step residual (add another half Dx to the previous half)
    TSparseSpace::Assign((*this->mpDx),1.0,Dx0);
    double rf = 0;
    ComputeUpdatedResidualNorm(rf);

    //compute optimal (limited to the range 0-1)
    //parabola is y = a*x^2 + b*x + c -> min/max for
    //x=0   --> r=ro
    //x=1/2 --> r=rh
    //x=1   --> r =
    //c= ro,     b= 4*rh -rf -3*ro,  a= 2*rf - 4*rh + 2*ro
    //max found if a>0 at the position  xmax = (rf/4 - rh)/(rf - 2*rh);
    double parabola_a = 2*rf + 2*ro - 4*rh;
    double parabola_b = 4*rh - rf - 3*ro;
    double xmin = 1e-3;
    double xmax = 1.0;
    double alpha = mAlpha;
    if( parabola_a > 0) //if parabola has a local minima
    {
      xmax = -0.5 * parabola_b/parabola_a; // -b / 2a
      if( xmax > 1.0)
        xmax = 1.0;
      else if(xmax < -1.0)
        xmax = -1.0;
    }
    else //parabola degenerates to either a line or to have a local max. best solution on either extreme
    {
      if(rf < ro)
        xmax = 1.0;
      else
        xmax = xmin; //should be zero, but otherwise it will stagnate
    }

    alpha  = -(1.0-xmax);
    mAlpha = alpha;
    
    //perform final update
    TSparseSpace::Assign((*this->mpDx),alpha,Dx0);
    BaseType::Update();

    //restore
    TSparseSpace::Assign((*this->mpDx),1.0,Dx0);

    KRATOS_CATCH( "" )
  }

  //**************************************************************************
  //**************************************************************************

  void Restore()
  {
    //Restore Current Displacement, Velocity, Acceleration
    TSparseSpace::InplaceMult((*this->mpDx),-1.0);
    BaseType::Update();
  }
  
  //**************************************************************************
  //**************************************************************************

  void ComputeSlope(double& rSlope, const SystemVectorType& rDx)
  {
    TSparseSpace::SetToZero((*this->mpb));
    this->mpBuilderAndSolver->BuildRHS(this->mpScheme, BaseType::GetModelPart(), (*this->mpb) );
    rSlope = TSparseSpace::Dot(rDx,(*this->mpb));
    Restore();    
  }

  //**************************************************************************
  //**************************************************************************
  
  void ComputeUpdatedSlope(double& rSlope, const SystemVectorType& rDx)
  {
    BaseType::Update();
    ComputeSlope(rSlope,rDx);
  }

  //**************************************************************************
  //**************************************************************************
  
  void ComputeResidualNorm(double& rNorm)
  {
    TSparseSpace::SetToZero((*this->mpb));
    this->mpBuilderAndSolver->BuildRHS(this->mpScheme, BaseType::GetModelPart(), (*this->mpb) );
    rNorm = TSparseSpace::TwoNorm((*this->mpb));
    Restore();    
  }
  
  //**************************************************************************
  //**************************************************************************

  void ComputeUpdatedResidualNorm(double& rNorm)
  {
    BaseType::Update();
    ComputeResidualNorm(rNorm);
  }

  
  ///@}
  ///@name Private  Access
  ///@{
  ///@}
  ///@name Private Inquiry
  ///@{
  ///@}
  ///@name Un accessible methods
  ///@{

  /// Copy constructor.
  LineSearchSolutionStrategy(const LineSearchSolutionStrategy& Other) {};

  ///@}

}; /// Class LineSearchSolutionStrategy

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

///@}

///@} addtogroup block
  
}  // namespace Kratos.

#endif // KRATOS_LINE_SEARCH_STRATEGY_H_INCLUDED
