#include <script/compiler/ast/AstVariableDeclaration.hpp>
#include <script/compiler/ast/AstUndefined.hpp>
#include <script/compiler/ast/AstTypeExpression.hpp>
#include <script/compiler/ast/AstEnumExpression.hpp>
#include <script/compiler/ast/AstTemplateExpression.hpp>
#include <script/compiler/AstVisitor.hpp>
#include <script/compiler/Keywords.hpp>
#include <script/compiler/Configuration.hpp>
#include <script/compiler/SemanticAnalyzer.hpp>

#include <script/compiler/type-system/BuiltinTypes.hpp>

#include <script/compiler/emit/Instruction.hpp>
#include <script/compiler/emit/BytecodeChunk.hpp>
#include <script/compiler/emit/BytecodeUtil.hpp>

#include <script/Instructions.hpp>
#include <system/Debug.hpp>
#include <util/UTF8.hpp>

#include <iostream>

namespace hyperion::compiler {

AstVariableDeclaration::AstVariableDeclaration(
    const std::string &name,
    const RC<AstPrototypeSpecification> &proto,
    const RC<AstExpression> &assignment,
    const Array<RC<AstParameter>> &template_params,
    IdentifierFlagBits flags,
    const SourceLocation &location
) : AstDeclaration(name, location),
    m_proto(proto),
    m_assignment(assignment),
    m_template_params(template_params),
    m_flags(flags)
{
}

void AstVariableDeclaration::Visit(AstVisitor *visitor, Module *mod)
{
    AstDeclaration::Visit(visitor, mod);

    if (m_identifier != nullptr) {
        m_identifier->GetFlags() |= m_flags;
    }

    SymbolTypePtr_t symbol_type;

    const bool has_user_assigned = m_assignment != nullptr;
    const bool has_user_specified_type = m_proto != nullptr;

    bool is_default_assigned = false;

    if (has_user_assigned) {
        m_real_assignment = m_assignment;
    }

    if (IsGeneric()) {
        mod->m_scopes.Open(Scope(SCOPE_TYPE_NORMAL, UNINSTANTIATED_GENERIC_FLAG));
    }

    if (!has_user_specified_type && !has_user_assigned) {
        // error; requires either type, or assignment.
        visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
            LEVEL_ERROR,
            Msg_missing_type_and_assignment,
            m_location,
            m_name
        ));
    } else {

        // flag that is turned on when there is no default type or assignment (an error)
        bool no_default_assignment = false;

        /* ===== handle type specification ===== */
        if (has_user_specified_type) {
            m_proto->Visit(visitor, mod);

            AssertThrow(m_proto->GetHeldType() != nullptr);
            symbol_type = m_proto->GetHeldType();

            if (m_identifier != nullptr) {
                m_identifier->SetSymbolType(symbol_type);
            }

#if ACE_ANY_ONLY_FUNCTION_PARAMATERS
            if (symbol_type->IsAnyType()) {
                // Any type is reserved for method parameters
                visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                    LEVEL_ERROR,
                    Msg_any_reserved_for_parameters,
                    m_location
                ));
            }
#endif

            // if no assignment provided, set the assignment to be the default value of the provided type
            if (m_real_assignment == nullptr) {
                // generic/non-concrete types that have default values
                // will get assigned to their default value without causing
                // an error
                if (const RC<AstExpression> default_value = m_proto->GetDefaultValue()) {
                    // Assign variable to the default value for the specified type.
                    m_real_assignment = CloneAstNode(default_value);
                    // built-in assignment, turn off strict mode
                    is_default_assigned = true;
                } else if (symbol_type->GetTypeClass() == TYPE_GENERIC) {
                    const bool no_parameters_required = symbol_type->GetGenericInfo().m_num_parameters == -1;

                    if (!no_parameters_required) {
                        // @TODO - idk. instantiate generic? it works for now, example being 'Function' type
                    } else {
                        // generic not yet instantiated; assignment does not fulfill enough to complete the type.
                        // since there is no assignment to go by, we can't promote it
                        visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                            LEVEL_ERROR,
                            Msg_generic_parameters_missing,
                            m_location,
                            symbol_type->GetName(),
                            symbol_type->GetGenericInfo().m_num_parameters
                        ));
                    }
                } else if (!symbol_type->IsGenericParameter() && !symbol_type->IsProxyClass()) { // generic parameters will be resolved upon instantiation
                    // no default assignment for this type
                    no_default_assignment = true;
                }
            }
        }

        if (m_real_assignment == nullptr) {
            // no assignment found - set to undefined (instead of a null pointer)
            m_real_assignment.Reset(new AstUndefined(m_location));
        }

        // Set identifier currentl value in advance,
        // so from type declarations we can use the current type.
        // Note: m_identifier will only be nullptr if declaration has failed.
        if (m_identifier != nullptr) {
            m_identifier->SetCurrentValue(m_real_assignment);
        }

        // if the variable has been assigned to an anonymous type,
        // rename the type to be the name of this variable
        if (AstTypeExpression *as_type_expr = dynamic_cast<AstTypeExpression *>(m_real_assignment.Get())) {
            as_type_expr->SetName(m_name);
        } else if (AstEnumExpression *as_enum_expr = dynamic_cast<AstEnumExpression *>(m_real_assignment.Get())) {
            as_enum_expr->SetName(m_name);
        } // @TODO more polymorphic way of doing this..

        // do this only within the scope of the assignment being visited.
        bool pass_by_ref_scope = false;
        bool pass_by_const_scope = false;

        if (IsConst()) {
            mod->m_scopes.Open(Scope(SCOPE_TYPE_NORMAL, CONST_VARIABLE_FLAG));

            pass_by_const_scope = true;
        }

        if (IsRef()) {
            if (has_user_assigned) {
                if (m_real_assignment->GetAccessOptions() & AccessMode::ACCESS_MODE_STORE) {
                    mod->m_scopes.Open(Scope(SCOPE_TYPE_NORMAL, REF_VARIABLE_FLAG));

                    pass_by_ref_scope = true;
                } else {
                    visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                        LEVEL_ERROR,
                        Msg_cannot_create_reference,
                        m_location
                    ));
                }
            } else {
                visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                    LEVEL_ERROR,
                    Msg_ref_missing_assignment,
                    m_location,
                    m_name
                ));
            }
        }

        // visit assignment
        if (m_name == "$construct") {//m_flags & FLAG_CONSTRUCTOR) {
            m_real_assignment->ApplyExpressionFlags(EXPR_FLAGS_CONSTRUCTOR_DEFINITION);
        }

        m_real_assignment->Visit(visitor, mod);

        /* ===== handle assignment ===== */
        // has received an explicit assignment
        // make sure type is compatible with the type.
        if (has_user_assigned) {
            AssertThrow(m_real_assignment->GetExprType() != nullptr);

            if (has_user_specified_type) {
                if (!is_default_assigned) { // default assigned is not typechecked
                    SemanticAnalyzer::Helpers::EnsureLooseTypeAssignmentCompatibility(
                        visitor,
                        mod,
                        symbol_type,
                        m_real_assignment->GetExprType(),
                        m_real_assignment->GetLocation()
                    );
                }
            } else {
                // Set the type to be the deduced type from the expression.
                // if (const AstTypeObject *real_assignment_as_type_object = dynamic_cast<const AstTypeObject*>(m_real_assignment->GetValueOf())) {
                //     symbol_type = real_assignment_as_type_object->GetHeldType();
                // } else {
                    symbol_type = m_real_assignment->GetExprType();
                // }
            }
        }

        // if (symbol_type == BuiltinTypes::ANY) {
        //     no_default_assignment = false;
        // }

        if (no_default_assignment) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_type_no_default_assignment,
                m_proto != nullptr
                    ? m_proto->GetLocation()
                    : m_location,
                symbol_type->GetName()
            ));
        }

        if (pass_by_ref_scope) {
            mod->m_scopes.Close();
        }

        if (pass_by_const_scope) {
            mod->m_scopes.Close();
        }
    }

    if (IsGeneric()) {
        // close template param scope
        mod->m_scopes.Close();
    }

    if (IsConst()) {
        if (!has_user_assigned && !is_default_assigned) {
            visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
                LEVEL_ERROR,
                Msg_const_missing_assignment,
                m_location,
                m_name
            ));
        }
    }

    if (symbol_type == nullptr) {
        visitor->GetCompilationUnit()->GetErrorList().AddError(CompilerError(
            LEVEL_ERROR,
            Msg_could_not_deduce_type_for_expression,
            m_location,
            m_name
        ));

        return;
    }

    if (m_identifier != nullptr) {
        m_identifier->SetSymbolType(symbol_type);
    }
}

std::unique_ptr<Buildable> AstVariableDeclaration::Build(AstVisitor *visitor, Module *mod)
{
    // if (m_is_generic) {
    //     // generics do not build anything
    //     return nullptr;
    // }

    std::unique_ptr<BytecodeChunk> chunk = BytecodeUtil::Make<BytecodeChunk>();

    AssertThrow(m_real_assignment != nullptr);

    if (!Config::cull_unused_objects || m_identifier->GetUseCount() > 0) {
        // update identifier stack location to be current stack size.
        m_identifier->SetStackLocation(visitor->GetCompilationUnit()->GetInstructionStream().GetStackSize());

        chunk->Append(m_real_assignment->Build(visitor, mod));

        // get active register
        UInt8 rp = visitor->GetCompilationUnit()->GetInstructionStream().GetCurrentRegister();

        { // add instruction to store on stack
            auto instr_push = BytecodeUtil::Make<RawOperation<>>();
            instr_push->opcode = PUSH;
            instr_push->Accept<UInt8>(rp);
            chunk->Append(std::move(instr_push));
        }

        { // add a comment for debugging to know where the var exists 
            chunk->Append(BytecodeUtil::Make<Comment>(" Var `" + m_name + "` at stack location: "
                + std::to_string(m_identifier->GetStackLocation())));
        }

        // increment stack size
        visitor->GetCompilationUnit()->GetInstructionStream().IncStackSize();
    } else {
        // if assignment has side effects but variable is unused,
        // compile the assignment in anyway.
        if (m_real_assignment->MayHaveSideEffects()) {
            chunk->Append(m_real_assignment->Build(visitor, mod));
        }
    }

    return chunk;
}

void AstVariableDeclaration::Optimize(AstVisitor *visitor, Module *mod)
{
    if (m_real_assignment != nullptr) {
        m_real_assignment->Optimize(visitor, mod);
    }
}

RC<AstStatement> AstVariableDeclaration::Clone() const
{
    return CloneImpl();
}

} // namespace hyperion::compiler
