#ifndef AST_TYPEOF_EXPRESSION
#define AST_TYPEOF_EXPRESSION

#include <string>

#include <script/compiler/ast/AstExpression.hpp>
#include <script/compiler/ast/AstPrototypeSpecification.hpp>
#include <script/compiler/ast/AstString.hpp>
#include <script/compiler/ast/AstTypeObject.hpp>
#include <script/compiler/type-system/SymbolType.hpp>

#define HYP_SCRIPT_TYPEOF_RETURN_OBJECT 0

namespace hyperion::compiler {

class AstTypeOfExpression : public AstPrototypeSpecification {
public:
    AstTypeOfExpression(
        const RC<AstExpression> &expr,
        const SourceLocation &location
    );
    virtual ~AstTypeOfExpression() = default;

    inline const RC<AstExpression> &GetExpr() const
        { return m_expr; }

    virtual void Visit(AstVisitor *visitor, Module *mod) override;
    virtual std::unique_ptr<Buildable> Build(AstVisitor *visitor, Module *mod) override;
    virtual void Optimize(AstVisitor *visitor, Module *mod) override;
    
    virtual RC<AstStatement> Clone() const override;

    virtual SymbolTypePtr_t GetExprType() const override;
  
    virtual const AstExpression *GetValueOf() const override;
    virtual const AstExpression *GetDeepValueOf() const override;
    
private:
    RC<AstExpression> m_expr;

#if !HYP_SCRIPT_TYPEOF_RETURN_OBJECT
    RC<AstExpression> m_string_expr;
#endif

    inline RC<AstTypeOfExpression> CloneImpl() const
    {
        return RC<AstTypeOfExpression>(new AstTypeOfExpression(
            CloneAstNode(m_expr),
            m_location
        ));
    }
};

} // namespace hyperion::compiler

#endif
