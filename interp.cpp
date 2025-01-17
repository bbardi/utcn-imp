// This file is part of the IMP project.

#include "interp.h"
#include "program.h"
#include <limits>
#include <iostream>



// -----------------------------------------------------------------------------
void Interp::Run()
{
  for (;;) {
    auto op = prog_.Read<Opcode>(pc_);
    //std::cout << op<<"\n";
    switch (op) {
      case Opcode::PUSH_FUNC: {
        Push(prog_.Read<size_t>(pc_));
        continue;
      }
      case Opcode::PUSH_PROTO: {
        Push(prog_.Read<RuntimeFn>(pc_));
        continue;
      }
      case Opcode::PUSH_INT: {
        Interp::Value val(prog_.Read<int64_t>(pc_));
        val.Kind = Value::Kind::INT;
        Push(val);
        continue;
      }
      case Opcode::PEEK: {
        auto idx = prog_.Read<unsigned>(pc_);
        Interp::Value val(*(stack_.rbegin() + idx));
        val.Kind = Value::Kind::INT;
        Push(val);
        continue;
      }
      case Opcode::POKE: {
          auto idx = prog_.Read<uint64_t>(pc_);
          auto val = Pop();
          *(stack_.rbegin()+idx) = val;
          Push(val);
          continue;
      }
      case Opcode::POP: {
        Pop();
        continue;
      }
      case Opcode::CALL: {
        auto callee = Pop();
        switch (callee.Kind) {
          case Value::Kind::PROTO: {
            (*callee.Val.Proto) (*this);
            continue;
          }
          case Value::Kind::ADDR: {
            Push(pc_);
            pc_ = callee.Val.Addr;
            continue;
          }
          case Value::Kind::INT: {
            throw RuntimeError("cannot call integer");
          }
        }
        continue;
      }
      case Opcode::ADD: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (rhs > 0 && lhs > INT64_MAX - rhs)
          throw RuntimeError("Integer overflow");
        if (rhs < 0 && lhs < INT64_MIN - rhs)
          throw RuntimeError("Integer underflow");
        Push(lhs + rhs);
        continue;
      }
      case Opcode::SUB: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (lhs < 0 && rhs > INT64_MAX + lhs)
          throw RuntimeError("Integer overflow");
        if (lhs > 0 && rhs < INT64_MIN + lhs)
          throw RuntimeError("Integer underflow");
        Push(rhs - lhs);
        continue;
      }
      case Opcode::MUL: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        uint64_t x = rhs*lhs;
        if(lhs != 0 && x / lhs != rhs)
          throw RuntimeError("Integer overflow");
        Push(rhs * lhs);
        continue;
      }
      case Opcode::DIV: {
            auto rhs = PopInt();
            auto lhs = PopInt();
            Push(lhs / rhs);
            continue;
      }
      case Opcode::EQUAL: {
          auto rhs = PopInt();
          auto lhs = PopInt();
          uint64_t eq = rhs == lhs;
          Interp::Value val(eq);
          val.Kind = Value::Kind::INT;
          Push(val);
          continue;
      }
      case Opcode::MOD: {
          auto rhs = PopInt();
          auto lhs = PopInt();
          Push(lhs%rhs);
          continue;
      }
      case Opcode::RET: {
        auto depth = prog_.Read<unsigned>(pc_);
        auto nargs = prog_.Read<unsigned>(pc_);
        auto v = Pop();
        stack_.resize(stack_.size() - depth);
        pc_ = PopAddr();
        stack_.resize(stack_.size() - nargs);
        Push(v);
        continue;
      }
      case Opcode::JUMP_FALSE: {
        auto cond = Pop();
        auto addr = prog_.Read<size_t>(pc_);
        if (!cond) {
          pc_ = addr;
        }
        continue;
      }
      case Opcode::JUMP: {
        pc_ = prog_.Read<size_t>(pc_);
        continue;
      }
      case Opcode::STOP: {
        return;
      }
    }
  }
}
