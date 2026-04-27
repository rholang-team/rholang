// #include "compiler/backend/codegen.hpp"

// #include <algorithm>
// #include <ranges>
// #include <unordered_map>
// #include <vector>

// #include "compiler/ir/instr.hpp"
// #include "compiler/ir/value.hpp"
// #include "utils/match.hpp"

// namespace backend {
// class ValueState {
//     // pointers should come first
//     std::vector<std::shared_ptr<ir::Value>> preallocated_;

//     std::vector<std::shared_ptr<ir::Value>> spilled_;

//     std::unordered_map<std::shared_ptr<ir::Value>, AddressExpression>
//         stackMapping_;

//     std::array<std::optional<std::shared_ptr<ir::Value>>, 16> registers_;

//     void spill(std::shared_ptr<ir::Value> v) {
//         spilled_.push_back(v);
//     }

//     void spillRegister(PhysicalRegister::Name reg) {
//         auto& v = registers_[std::to_underlying(reg)];
//         spill(v.value());
//         v.reset();
//     }

//     bool isRegisterFree(PhysicalRegister::Name reg) const {
//         return !registers_[std::to_underlying(reg)].has_value();
//     }

//     bool isRegisterFree(const PhysicalRegister& reg) const {
//         return isRegisterFree(reg.name());
//     }

//     std::optional<PhysicalRegister::Name> findFreeRegister() const {
//         static constexpr std::array<PhysicalRegister::Name, 9> tmpRegs{
//             PhysicalRegister::Name::Rax,
//             PhysicalRegister::Name::Rcx,
//             PhysicalRegister::Name::Rdx,
//             PhysicalRegister::Name::Rdi,
//             PhysicalRegister::Name::Rsi,
//             PhysicalRegister::Name::R8,
//             PhysicalRegister::Name::R9,
//             PhysicalRegister::Name::R10,
//             PhysicalRegister::Name::R11,
//         };

//         auto it = std::ranges::find_if(tmpRegs, [this](auto&& r) {
//             return isRegisterFree(r);
//         });
//         if (it == tmpRegs.end()) {
//             return std::nullopt;
//         }
//         return *it;
//     }

// public:
//     ValueState() {}

//     void preallocate(/* TODO */) {}

//     void clear() {
//         preallocated_.clear();
//         spilled_.clear();
//         stackMapping_.clear();
//         std::ranges::for_each(registers_, [](auto& r) { r.reset(); });
//     }

//     // std::shared_ptr<ir::Value> fill() {
//     //     assert(!spilled_.empty());

//     //     ir::Type* top = spilled_.back();
//     //     spilled_.pop_back();
//     //     return top;
//     // }

//     void putIntoRegister(PhysicalRegister::Name reg,
//                          std::shared_ptr<ir::Value> value) {
//         if (!isRegisterFree(reg)) {
//             spillRegister(reg);
//         }

//         registers_[std::to_underlying(reg)] = value;
//     }

//     void putIntoRegister(const PhysicalRegister& reg,
//                          std::shared_ptr<ir::Value> value) {
//         putIntoRegister(reg.name(), value);
//     }

//     std::shared_ptr<PhysicalRegister> putIntoRegister(
//         std::shared_ptr<ir::Value> value) {
//         // TODO: probably find a better way to choose the register to spill
//         auto freeRegister = findFreeRegister();

//         PhysicalRegister::Name reg;
//         if (!freeRegister.has_value()) {
//             spillRegister();
//         }

//         return std::make_shared<PhysicalRegister>(reg);
//     }

//     PhysicalRegister getRegOrFill(std::shared_ptr<ir::Value> value) {}

//     // AddressExpression addr(std::shared_ptr<ir::Value> value) const {
//     //     return stackMapping_.at(value);
//     // }

//     // Register reg(std::shared_ptr<ir::Value> value) const {
//     //     return registerMaping_.at(value);
//     // }
// };

// class Codegen {
// public:
//     Codegen() = default;

//     Binary run(const ir::Module& mod) {
//         Binary res;

//         /* TODO: globals */

//         for (const auto& [name, fn] : mod.functions()) {
//             genFunction(name, *fn);
//         }

//         return res;
//     }

// private:
//     ValueState valueState_;

//     static size_t calculateFrameSize(const ir::Function& fn) {
//         size_t res = 0;

//         for (const auto& bb : fn) {
//             for (const auto& i : *bb) {
//                 if (!utils::isa<ir::AllocaInstr>(i.get()))
//                     continue;

//                 const ir::Type* itemType =
//                     static_cast<const ir::AllocaInstr&>(*i).itemType();

//                 res += getValueSize(itemType);
//             }
//         }

//         return res;
//     }

//     Function genFunction(const std::string& name, const ir::Function& fn) {
//         valueState_.clear();

//         Function res{name};

//         size_t stackFrameSize =
//             getTotalSizeWithAlignment(fn.signature()->type()->params()) +
//             calculateFrameSize(fn);
//         if (stackFrameSize > 0) {
//             BasicBlock prologue{};

//             prologue.addInstr(std::make_unique<SubInstr>(
//                 PhysicalRegister{PhysicalRegister::Name::Rsp},
//                 std::make_unique<Immediate>(stackFrameSize)));

//             res.addBb(std::move(prologue));
//         }

//         for (const auto& bb : fn) {
//             res.addBb(genBasicBlock(*bb));
//         }

//         return res;
//     }

//     BasicBlock genBasicBlock(const ir::BasicBlock& bb) {
//         assert(bb.hasTerminator());

//         BasicBlock res;

//         auto it = bb.begin();
//         for (;;) {
//             const ir::Instr* i = it->get();

//             genInstr(res, *i);

//             if (i->isTerminator()) {
//                 break;
//             }
//         }

//         return res;
//     }

//     void genInstr(BasicBlock& addTo, const ir::Instr& i) {
//         if (const ir::NewInstr* allocaInstr =
//                 dynamic_cast<const ir::NewInstr*>(&i))
//             genNewInstr(addTo, *allocaInstr);
//         else if (const ir::CallInstr* callInstr =
//                      dynamic_cast<const ir::CallInstr*>(&i))
//             genCallInstr(addTo, *callInstr);
//         else if (const ir::NotInstr* notInstr =
//                      dynamic_cast<const ir::NotInstr*>(&i))
//             genNotInstr(addTo, *notInstr);
//         else if (const ir::NegInstr* negInstr =
//                      dynamic_cast<const ir::NegInstr*>(&i))
//             genNegInstr(addTo, *negInstr);
//         else if (const ir::LoadInstr* loadInstr =
//                      dynamic_cast<const ir::LoadInstr*>(&i))
//             genLoadInstr(addTo, *loadInstr);
//         else if (const ir::StoreInstr* storeInstr =
//                      dynamic_cast<const ir::StoreInstr*>(&i))
//             genStoreInstr(addTo, *storeInstr);
//         else if (const ir::AddInstr* addInstr =
//                      dynamic_cast<const ir::AddInstr*>(&i))
//             genAddInstr(addTo, *addInstr);
//         else if (const ir::SubInstr* subInstr =
//                      dynamic_cast<const ir::SubInstr*>(&i))
//             genSubInstr(addTo, *subInstr);
//         else if (const ir::MulInstr* mulInstr =
//                      dynamic_cast<const ir::MulInstr*>(&i))
//             genMulInstr(addTo, *mulInstr);
//         else if (const ir::DivInstr* divInstr =
//                      dynamic_cast<const ir::DivInstr*>(&i))
//             genDivInstr(addTo, *divInstr);
//         else if (const ir::CmpInstr* cmpInstr =
//                      dynamic_cast<const ir::CmpInstr*>(&i))
//             genCmpInstr(addTo, *cmpInstr);
//         else if (const ir::GetFieldPtrInstr* getFieldPtrInstr =
//                      dynamic_cast<const ir::GetFieldPtrInstr*>(&i))
//             genGetFieldPtrInstr(addTo, *getFieldPtrInstr);
//         else if (const ir::GotoInstr* gotoInstr =
//                      dynamic_cast<const ir::GotoInstr*>(&i))
//             genGotoInstr(addTo, *gotoInstr);
//         else if (const ir::BrInstr* brInstr =
//                      dynamic_cast<const ir::BrInstr*>(&i))
//             genBrInstr(addTo, *brInstr);
//         else if (const ir::RetInstr* retInstr =
//                      dynamic_cast<const ir::RetInstr*>(&i))
//             genRetInstr(addTo, *retInstr);
//     }

//     void genNewInstr(BasicBlock& addTo, const ir::NewInstr& i) {}

//     void genCallInstr(BasicBlock& addTo, const ir::CallInstr& i) {}

//     void genNotInstr(BasicBlock& addTo, const ir::NotInstr& i) {}

//     void genNegInstr(BasicBlock& addTo, const ir::NegInstr& i) {}

//     void genLoadInstr(BasicBlock& addTo, const ir::LoadInstr& i) {}

//     void genStoreInstr(BasicBlock& addTo, const ir::StoreInstr& i) {}

//     void genAddInstr(BasicBlock& addTo, const ir::AddInstr& i) {}

//     void genSubInstr(BasicBlock& addTo, const ir::SubInstr& i) {}

//     void genMulInstr(BasicBlock& addTo, const ir::MulInstr& i) {}

//     void genDivInstr(BasicBlock& addTo, const ir::DivInstr& i) {}

//     void genGetFieldPtrInstr(BasicBlock& addTo, const ir::GetFieldPtrInstr& i) {
//     }

//     void genCmpInstr(BasicBlock& addTo, const ir::CmpInstr& i) {}

//     void genGotoInstr(BasicBlock& addTo, const ir::GotoInstr& i) {}

//     void genBrInstr(BasicBlock& addTo, const ir::BrInstr& i) {}

//     void genRetInstr(BasicBlock& addTo, const ir::RetInstr& i) {}
// };
// }  // namespace

// Binary codegen(ir::Module& module) {
//     return Codegen{}.run(module);
// }
// }  // namespace backend
