#include <limits>

#include "TMVA/RModel.hxx"




namespace TMVA{
namespace Experimental{
namespace SOFIE{

   RModel::RModel(RModel&& other){
      fInputTensorInfos = std::move(other.fInputTensorInfos);
      fReadyInputTensorInfos = std::move(other.fReadyInputTensorInfos);
      fOutputTensorNames = other.fOutputTensorNames;
      fOperators = std::move(other.fOperators);
      fInitializedTensors = std::move(other.fInitializedTensors);
      fIntermediateTensorInfos = std::move(other.fIntermediateTensorInfos);
      fName = other.fName;
      fFileName = other.fFileName;
      fParseTime = other.fParseTime;
      fGC = other.fGC;
      fNeededBlasRoutines = other.fNeededBlasRoutines;
      fNeededStdLib = other.fNeededStdLib;
   }

   RModel& RModel::operator=(RModel&& other){
      fInputTensorInfos = std::move(other.fInputTensorInfos);
      fReadyInputTensorInfos = std::move(other.fReadyInputTensorInfos);
      fOutputTensorNames = other.fOutputTensorNames;
      fOperators = std::move(other.fOperators);
      fInitializedTensors = std::move(other.fInitializedTensors);
      fIntermediateTensorInfos = std::move(other.fIntermediateTensorInfos);
      fName = other.fName;
      fFileName = other.fFileName;
      fParseTime = other.fParseTime;
      fGC = other.fGC;
      fNeededBlasRoutines = other.fNeededBlasRoutines;
      fNeededStdLib = other.fNeededStdLib;
      return *this;
   }

   RModel::RModel(std::string name, std::string parsedtime): fFileName (name), fParseTime(parsedtime) {
      fName = fFileName.substr(0, fFileName.rfind("."));
   }

   const std::vector<size_t>& RModel::GetTensorShape(std::string name){
      auto f = fReadyInputTensorInfos.find(name);
      if (f != fReadyInputTensorInfos.end()){
         return f->second.shape;
      }
      auto f2 = fInitializedTensors.find(name);
      if (f2 != fInitializedTensors.end()){
         return f2->second.fShape;
      }
      auto f3 = fInputTensorInfos.find(name);
      if (f3 != fInputTensorInfos.end()){
         throw std::runtime_error("TMVA SOFIE tensor [" + name + "] is an input tensor with unspecified dimension parameter");
      }
      auto f4 = fIntermediateTensorInfos.find(name);
      if (f4 != fIntermediateTensorInfos.end()){
         return f4->second.shape;
      }

      throw std::runtime_error("TMVA SOFIE tensor [" + name + "] for which the shape is requested is not found");
   }

   const ETensorType& RModel::GetTensorType(std::string name){
      auto f = fReadyInputTensorInfos.find(name);
      if (f != fReadyInputTensorInfos.end()){
         return f->second.type;
      }
      auto f2 = fInitializedTensors.find(name);
      if (f2 != fInitializedTensors.end()){
         return f2->second.fType;
      }
      auto f3 = fInputTensorInfos.find(name);
      if (f3 != fInputTensorInfos.end()){
         return f3->second.type;
      }
      auto f4 = fIntermediateTensorInfos.find(name);
      if (f4 != fIntermediateTensorInfos.end()){
         return f4->second.type;
      }

      throw std::runtime_error("TMVA SOFIE tensor [" + name + "] for which the shape is requested is not found");
   }

   bool RModel::CheckIfTensorAlreadyExist(std::string tensor_name){
      if (fReadyInputTensorInfos.find(tensor_name) != fReadyInputTensorInfos.end())  return true;
      if (fInitializedTensors.find(tensor_name) != fInitializedTensors.end()) return true;
      if (fIntermediateTensorInfos.find(tensor_name) != fIntermediateTensorInfos.end()) return true;
      return false;
   }

   void RModel::AddInputTensorInfo(std::string input_name, ETensorType type, std::vector<Dim> shape){
      input_name = UTILITY::Clean_name(input_name);
      if (CheckIfTensorAlreadyExist(input_name)){
         throw std::runtime_error("TMVA-SOFIE: input tensor with name " + input_name + " already exists \n");
      }

      InputTensorInfo inputInfo { type, shape };
      fInputTensorInfos[input_name] = inputInfo;
   }

   void RModel::AddInputTensorInfo(std::string input_name, ETensorType type, std::vector<size_t> shape){
      input_name = UTILITY::Clean_name(input_name);
      if (CheckIfTensorAlreadyExist(input_name)){
         throw std::runtime_error("TMVA-SOFIE: input tensor with name " + input_name + " already exists \n");
      }
      TensorInfo inputInfo { type, shape };
      fReadyInputTensorInfos[input_name] = inputInfo;
   }

   void RModel::AddOperator(std::unique_ptr<ROperator> op, int order_execution){
      if (order_execution >= 0) {
         fOperators.insert(fOperators.begin() + order_execution, std::move(op));
      }else{
         fOperators.push_back(std::move(op));
      }
   }

   void RModel::AddInitializedTensor(std::string tensor_name, ETensorType type, std::vector<std::size_t> shape, std::shared_ptr<void> data){
      tensor_name = UTILITY::Clean_name(tensor_name);
      //NB: own data
      if (CheckIfTensorAlreadyExist(tensor_name)){
         throw std::runtime_error("TMVA-SOFIE: initialized tensor with name " + tensor_name + " already exists \n");
      }
      InitializedTensor new_tensor {type, shape, data};
      fInitializedTensors[tensor_name] = new_tensor;

   }

   void RModel::AddIntermediateTensor(std::string tensor_name, ETensorType type, std::vector<std::size_t> shape){
      tensor_name = UTILITY::Clean_name(tensor_name);
      if (CheckIfTensorAlreadyExist(tensor_name)){
         throw std::runtime_error("TMVA-SOFIE: intermediate tensor with name " + tensor_name + " already exists \n");
      }
      TensorInfo new_tensor {type, shape};
      fIntermediateTensorInfos[tensor_name] = new_tensor;
   }

   void RModel::AddOutputTensorNameList(std::vector<std::string> outputtensornames){
      for(auto& it : outputtensornames){
         fOutputTensorNames.push_back(UTILITY::Clean_name(it));
      }
   }

   void RModel::UpdateInitializedTensor(std::string tensor_name, ETensorType type, std::vector<std::size_t> shape, std::shared_ptr<void> data){
      tensor_name = UTILITY::Clean_name(tensor_name);
      if (not CheckIfTensorAlreadyExist(tensor_name)){
         throw std::runtime_error("TMVA-SOFIE: tensor " + tensor_name + " not found when trying to update it");
      }
      InitializedTensor new_tensor {type, shape, data};
      fInitializedTensors[tensor_name] = new_tensor;
   }

   std::shared_ptr<void> RModel::GetInitializedTensorData(std::string tensor_name){
      auto f = fInitializedTensors.find(tensor_name);
      if (f == fInitializedTensors.end()){
         throw std::runtime_error("TMVA-SOFIE: tensor " + tensor_name + " not found when trying to get its data");
      }else{
         return f->second.fData;
      }
   }

   void RModel::Initialize(){
      for (auto& i : fOperators){
         i->Initialize(*this);
      }
   }

   void RModel::Generate(){
      Initialize();
      fGC += ("//Code generated automatically by TMVA for Inference of Model file [" + fFileName + "] at [" + fParseTime.substr(0, fParseTime.length()-1) +"] \n");
      for (auto& i: fNeededStdLib) {
         fGC += "#include<" + i + ">\n";
      }
      fGC += ("namespace TMVA_SOFIE_" + fName + "{\n");
      if (!fNeededBlasRoutines.empty()) {
         fGC += ("namespace BLAS{\n");
         for (auto &routine : fNeededBlasRoutines) {
            if (routine == "Gemm") {
               fGC += ("\textern \"C\" void sgemm_(const char * transa, const char * transb, const int * m, const int * n, const int * k,\n"
                       "\t                       const float * alpha, const float * A, const int * lda, const float * B, const int * ldb,\n"
                       "\t                       const float * beta, float * C, const int * ldc);\n");
            } else if (routine == "Gemv") {
               fGC += ("\textern \"C\" void sgemv_(const char * trans, const int * m, const int * n, const float * alpha, const float * A,\n"
                       "\t                       const int * lda, const float * X, const int * incx, const float * beta, const float * Y, const int * incy);\n");
            } else if (routine == "Axpy") {
               fGC += ("\textern \"C\" void saxpy_(const int * n, const float * alpha, const float * x,\n"
                       "\t                         const int * incx, float * y, const int * incy);\n");
            }
         }
         fGC += ("}//BLAS\n");
      }
      for (auto& i: fInitializedTensors){
         if (i.second.fType == ETensorType::FLOAT){
            size_t length = 1;
            for (auto & dim: i.second.fShape){
               length *= dim;
            }
            fGC += "float tensor_" + i.first + "[" + std::to_string(length) + "] = {";
            std::shared_ptr<float> data = std::static_pointer_cast<float>(i.second.fData);
            std::stringstream floats;
            for (size_t idx = 0; idx < length-1; idx++){
               floats << std::setprecision(std::numeric_limits<float>::max_digits10) << data.get()[idx] << ", ";
            }
            floats << std::setprecision(std::numeric_limits<float>::max_digits10) << data.get()[length-1];
            fGC += floats.str() +"};\n";
         }
      }
      for (auto&i: fIntermediateTensorInfos){
         if (i.second.type == ETensorType::FLOAT){
            size_t length = 1;
            for (auto & dim: i.second.shape){
               length *= dim;
            }
            fGC += "float tensor_" + i.first + "[" + std::to_string(length) + "];\n";
         }
      }

      size_t outputSize = fOutputTensorNames.size();
      if (outputSize == 1) {
         auto f = fIntermediateTensorInfos.find(fOutputTensorNames[0]);
         if (f == fIntermediateTensorInfos.end()){
            throw std::runtime_error("TMVA-SOFIE: output tensor " + fOutputTensorNames[0] + " not found when trying to get its info");
         }else{
            if (f->second.type == ETensorType::FLOAT){
               fGC += "std::vector<float> ";
            }
         }
      } else {
         std::vector<ETensorType> outputTensorsTypes(outputSize);
         for (size_t i = 0; i < outputSize; i++) {
            auto f = fIntermediateTensorInfos.find(fOutputTensorNames[i]);
            if (f == fIntermediateTensorInfos.end()) {
               throw std::runtime_error("TMVA-SOFIE: output tensor " + fOutputTensorNames[i]
                  + " not found when trying to get its info");
            } else {
               outputTensorsTypes[i] = f->second.type;
            }
         }
         ETensorType outputType = outputTensorsTypes[0];
         for (size_t i = 0; i < outputSize; i++) {
            if (outputTensorsTypes[i] != outputType) {
               throw std::runtime_error("TMVA-SOFIE: output tensor " + fOutputTensorNames[i] + " is of different type.");
            }
         }
         if (outputType == ETensorType::FLOAT) {
            fGC += "std::vector<std::vector<float>> ";
         }
      }

      fGC += "infer(";
      for (auto& i: fReadyInputTensorInfos){
         size_t length = 1;
         for (auto& dim: i.second.shape){
            length *= dim;
         }
         if (i.second.type == ETensorType::FLOAT){
         fGC += "float* tensor_" + i.first + ",";
         }
      }
      fGC.pop_back(); //remove last ","
      fGC += "){\n";

      for (size_t id = 0; id < fOperators.size() ; id++){
         fGC+= (fOperators[id]->Generate(std::to_string(id)));
      }
      if (outputSize == 1) {
         fGC += "\tstd::vector<float> ret (tensor_" + fOutputTensorNames[0] + ", tensor_" + fOutputTensorNames[0] + " + sizeof(tensor_" +
               fOutputTensorNames[0] + ") / sizeof(tensor_" + fOutputTensorNames[0] + "[0]));\n";
      } else {
         for (size_t i = 0; i < outputSize; i++) {
            if (!fOutputTensorNames[i].empty()) {
               fGC += "\tstd::vector<float> ret_";
               fGC += std::to_string(i);
               fGC += " (tensor_" + fOutputTensorNames[i] + ", tensor_" + fOutputTensorNames[i] + " + sizeof(tensor_" + fOutputTensorNames[i] + ") / sizeof(tensor_" + fOutputTensorNames[i] + "[0]));\n";
            }
         }
         fGC += "\tstd::vector<std::vector<float>> ret({";
         for (size_t i = 0; i < outputSize; i++) {
            if (fOutputTensorNames[i].empty()) {
               fGC += "{}";
            } else {
               fGC += "ret_";
               fGC += std::to_string(i);
            }
            if (i < outputSize - 1) {
               fGC += ",";
            }
         }
         fGC += "});\n";
      }
      fGC += "\treturn ret;\n";
      fGC += "}\n";
      fGC += ("} //TMVA_SOFIE_" + fName + "\n");
   }



   void RModel::PrintRequiredInputTensors(){
      std::cout << "Model requires following inputs:\n";
      for (auto& inputInfo: fInputTensorInfos){
         std::cout << "Parameterised Tensor name: " << inputInfo.first << "\t";
         std::cout << "type: " << ConvertTypeToString(inputInfo.second.type) << "\t";
         std::cout << "shape: [";
         for (size_t i = 0; i < inputInfo.second.shape.size(); i++){
            if (inputInfo.second.shape[i].isParam){
               std::cout << inputInfo.second.shape[i].param;
            }else{
               std::cout << inputInfo.second.shape[i].dim ;
            }
            if (i < inputInfo.second.shape.size() - 1) std::cout << ",";
         }
         std::cout << "]" << std::endl;
      }

      for (auto& inputInfo: fReadyInputTensorInfos){
         std::cout << "Fully Specified Tensor name: " << inputInfo.first << "\t";
         std::cout << "type: " << ConvertTypeToString(inputInfo.second.type) << "\t";
         std::cout << "shape: [";
         for (size_t i = 0; i < inputInfo.second.shape.size(); i++){
            std::cout << inputInfo.second.shape[i];
            if (i < inputInfo.second.shape.size() - 1) std::cout << ",";
         }
         std::cout << "]" << std::endl;
      }

   }

   void RModel::PrintInitializedTensors(){
      std::cout << "Model initialized the following tensors:\n";
      for (auto& it: fInitializedTensors){
         std::cout << "Tensor name: \"" << it.first << "\"\t";
         std::cout << "type: " << ConvertTypeToString(it.second.fType) << "\t";
         std::cout << "shape: [";
         for (size_t i = 0; i < it.second.fShape.size(); i++){
            std::cout << it.second.fShape[i];
            if (i < it.second.fShape.size() - 1) std::cout << ",";
         }
         std::cout << "]" << std::endl;
      }
   }

   void RModel::PrintIntermediateTensors(){
      std::cout << "Model specify the following intermediate tensors:\n";
      for (auto& it: fIntermediateTensorInfos){
         std::cout << "Tensor name: \"" << it.first << "\"\t";
         std::cout << "type: " << ConvertTypeToString(it.second.type) << "\t";
         std::cout << "shape: [";
         for (size_t i = 0; i < it.second.shape.size(); i++){
            std::cout << it.second.shape[i];
            if (i < it.second.shape.size() - 1) std::cout << ",";
         }
         std::cout << "]" << std::endl;
      }
   }

   void RModel::HeadInitializedTensors(std::string name, int n_print){
      auto it = fInitializedTensors.find(name);
      if (it == fInitializedTensors.end()){
         std::cout << "Tensor " << name << " not found in model's intiialized tensor list" << std::endl;
         return;
      }

      std::cout << "Tensor name: " << it->first << "\t";
      std::cout << "type: " << ConvertTypeToString(it->second.fType) << "\t";
      int length =1;
      std::cout << "shape: [";
      for (size_t i = 0; i < it->second.fShape.size(); i++){
         std::cout << it->second.fShape[i];
         length *= it->second.fShape[i];
         if (i < it->second.fShape.size() - 1) std::cout << ",";
      }
      std::cout << "]" << std::endl;
      bool ellipsis = true;
      if (n_print > length){
         n_print = length;
         ellipsis = false;
      }

      std::cout << "data: [" << std::endl;
      //switch(it->second.type){
      //   case ETensorType::FLOAT : {
      if (it->second.fType == ETensorType::FLOAT) {
         auto converted_data = std::static_pointer_cast<float>(it->second.fData).get();
         for (int i =0; i < n_print; i++){
            std::cout << converted_data[i];
            if (i < n_print - 1) std::cout << " ,";
         }
         //   break;
         // }
      }
      if (ellipsis) std::cout << ", ...";
      std::cout << "]" << std::endl;

   }

   void RModel::OutputGenerated(std::string filename){
      if (filename == ""){
         filename = fName + ".hxx";
      }
      std::ofstream f;
      f.open(filename);
      if (!f.is_open()){
         throw std::runtime_error("tmva-sofie failed to open file for output generated inference code");
      }
      f << fGC;
      f.close();
   }

   void RModel::Streamer(TBuffer &R__b){
       if (R__b.IsReading()) {
           RModel::Class()->ReadBuffer(R__b, this);
           for(auto i=RModel::fInitializedTensors.begin(); i!=RModel::fInitializedTensors.end();++i){
               i->second.CastPersistentToShared();
           }
       }
       else {
          for(auto i=RModel::fInitializedTensors.begin(); i!=RModel::fInitializedTensors.end();++i){
               i->second.CastSharedToPersistent();
           }
          RModel::Class()->WriteBuffer(R__b, this);
       }
   }

}//SOFIE
}//Experimental
}//TMVA
