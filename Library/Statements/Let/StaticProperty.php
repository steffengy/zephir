<?php

/*
 +--------------------------------------------------------------------------+
 | Zephir Language                                                          |
 +--------------------------------------------------------------------------+
 | Copyright (c) 2013-2015 Zephir Team and contributors                     |
 +--------------------------------------------------------------------------+
 | This source file is subject the MIT license, that is bundled with        |
 | this package in the file LICENSE, and is available through the           |
 | world-wide-web at the following url:                                     |
 | http://zephir-lang.com/license.html                                      |
 |                                                                          |
 | If you did not receive a copy of the MIT license and are unable          |
 | to obtain it through the world-wide-web, please send a note to           |
 | license@zephir-lang.com so we can mail you a copy immediately.           |
 +--------------------------------------------------------------------------+
*/

namespace Zephir\Statements\Let;

use Zephir\CompilationContext;
use Zephir\CompilerException;
use Zephir\Variable as ZephirVariable;
use Zephir\Detectors\ReadDetector;
use Zephir\Expression;
use Zephir\CompiledExpression;
use Zephir\Compiler;
use Zephir\Utils;
use Zephir\GlobalConstant;

/**
 * StaticProperty
 *
 * Updates static properties
 */
class StaticProperty
{
    /**
     * Compiles ClassName::foo = {expr}
     *
     * @param                    $className
     * @param                    $property
     * @param CompiledExpression $resolvedExpr
     * @param CompilationContext $compilationContext
     * @param array              $statement
     *
     * @throws CompilerException
     * @internal param string $variable
     */
    public function assignStatic($className, $property, CompiledExpression $resolvedExpr, CompilationContext $compilationContext, $statement)
    {
        $compiler = $compilationContext->compiler;
        if (!in_array($className, array('self', 'static', 'parent'))) {
            $className = $compilationContext->getFullName($className);
            if ($compiler->isClass($className)) {
                $classDefinition = $compiler->getClassDefinition($className);
            } else {
                if ($compiler->isInternalClass($className)) {
                    $classDefinition = $compiler->getInternalClassDefinition($className);
                } else {
                    throw new CompilerException("Cannot locate class '" . $className . "'", $statement);
                }
            }
        } else {
            if (in_array($className, array('self', 'static'))) {
                $classDefinition = $compilationContext->classDefinition;
            } else {
                if ($className == 'parent') {
                    $classDefinition = $compilationContext->classDefinition;
                    $extendsClass = $classDefinition->getExtendsClass();
                    if (!$extendsClass) {
                        throw new CompilerException('Cannot assign static property "' . $property . '" on parent because class ' . $classDefinition->getCompleteName() . ' does not extend any class', $statement);
                    } else {
                        $classDefinition = $classDefinition->getExtendsClassDefinition();
                    }
                }
            }
        }

        if (!$classDefinition->hasProperty($property)) {
            throw new CompilerException("Class '" . $classDefinition->getCompleteName() . "' does not have a property called: '" . $property . "'", $statement);
        }

        /** @var $propertyDefinition ClassProperty */
        $propertyDefinition = $classDefinition->getProperty($property);
        if (!$propertyDefinition->isStatic()) {
            throw new CompilerException("Cannot access non-static property '" . $classDefinition->getCompleteName() . '::' . $property . "'", $statement);
        }

        if ($propertyDefinition->isPrivate()) {
            if ($classDefinition != $compilationContext->classDefinition) {
                throw new CompilerException("Cannot access private static property '" . $classDefinition->getCompleteName() . '::' . $property . "' out of its declaring context", $statement);
            }
        }

        $codePrinter = $compilationContext->codePrinter;

        $compilationContext->headersManager->add('kernel/object');
        $classEntry = $classDefinition->getClassEntry($compilationContext);

        switch ($resolvedExpr->getType()) {

            case 'null':
                $codePrinter->output('zephir_update_static_property_null(' . $classEntry . ', SL("' . $property . '"));');
                break;

            case 'int':
            case 'uint':
            case 'long':
                $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                $codePrinter->output('ZVAL_LONG(' . $tempVariable->getPointeredName() . ', ' . $resolvedExpr->getCode() . ');');
                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                if ($tempVariable->isTemporal()) {
                    $tempVariable->setIdle(true);
                }
                break;

            case 'char':
            case 'uchar':
                $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                $codePrinter->output('ZVAL_LONG(' . $tempVariable->getPointeredName() . ', \'' . $resolvedExpr->getCode() . '\');');
                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                if ($tempVariable->isTemporal()) {
                    $tempVariable->setIdle(true);
                }
                break;

            case 'double':
                $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                $codePrinter->output('ZVAL_DOUBLE(' . $tempVariable->getPointeredName() . ', ' . $resolvedExpr->getCode() . ');');
                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                if ($tempVariable->isTemporal()) {
                    $tempVariable->setIdle(true);
                }
                break;

            case 'string':
                switch ($statement['operator']) {
                    case 'assign':
                        $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                        $tempVariable->initVariant($compilationContext);

                        if ($resolvedExpr->getCode()) {
                            $codePrinter->output('ZVAL_STRING(&' . $tempVariable->getName() . ', "' . $resolvedExpr->getCode() . '");');
                        } else {
                            $codePrinter->output('ZVAL_EMPTY_STRING(&' . $tempVariable->getName() . ');');
                        }

                        if ($tempVariable->isTemporal()) {
                            $tempVariable->setIdle(true);
                        }
                        break;
                    default:
                        throw new CompilerException("Operator '" . $statement['operator'] . "' is not supported for variable type: string", $statement);
                }

                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                break;

            case 'bool':
                $codePrinter->output('zephir_update_static_property_bool(' . $classEntry . ', SL("' . $property . '"), ' . $resolvedExpr->getBooleanCode() . ');');
                break;

            case 'empty-array':
                $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                $codePrinter->output('array_init(' . $tempVariable->getPointeredName() . ');');
                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                if ($tempVariable->isTemporal()) {
                    $tempVariable->setIdle(true);
                }
                break;

            case 'array':
                $variableVariable = $compilationContext->symbolTable->getVariableForRead($resolvedExpr->getCode(), $compilationContext, $statement);
                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $variableVariable->getName() . ' TSRMLS_CC);');
                break;

            case 'variable':
                $variableVariable = $compilationContext->symbolTable->getVariableForRead($resolvedExpr->getCode(), $compilationContext, $statement);
                switch ($variableVariable->getType()) {

                    case 'int':
                    case 'uint':
                    case 'long':
                    case 'ulong':
                    case 'char':
                    case 'uchar':
                        $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                        $codePrinter->output('ZVAL_LONG(' . $tempVariable->getPointeredName() . ', ' . $variableVariable->getName() . ');');
                        if ($compilationContext->insideCycle) {
                            $propertyCache = $compilationContext->symbolTable->getTempVariableForWrite('zend_property_info', $compilationContext);
                            $propertyCache->setMustInitNull(true);
                            $propertyCache->setReusable(false);
                            $codePrinter->output('zephir_update_static_property_ce_cache(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ', &' . $propertyCache->getName() . ' TSRMLS_CC);');
                        } else {
                            $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), ' . $tempVariable->getPointeredName() . ');');
                        }
                        if ($tempVariable->isTemporal()) {
                            $tempVariable->setIdle(true);
                        }
                        break;

                    case 'double':
                        $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                        $codePrinter->output('ZVAL_DOUBLE(' . $tempVariable->getPointeredName() . ', ' . $variableVariable->getName() . ');');
                        if ($compilationContext->insideCycle) {
                            $propertyCache = $compilationContext->symbolTable->getTempVariableForWrite('zend_property_info', $compilationContext);
                            $propertyCache->setMustInitNull(true);
                            $propertyCache->setReusable(false);
                            $codePrinter->output('zephir_update_static_property_ce_cache(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ', &' . $propertyCache->getName() . ' TSRMLS_CC);');
                        } else {
                            $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                        }
                        if ($tempVariable->isTemporal()) {
                            $tempVariable->setIdle(true);
                        }
                        break;

                    case 'bool':
                        $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                        $codePrinter->output('ZVAL_BOOL(' . $tempVariable->getPointeredName() . ', ' . $variableVariable->getName() . ');');
                        $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $tempVariable->getName() . ' TSRMLS_CC);');
                        if ($tempVariable->isTemporal()) {
                            $tempVariable->setIdle(true);
                        }
                        break;

                    case 'string':
                        switch ($statement['operator']) {
                            case 'concat-assign':
                                $tempVariable = $compilationContext->symbolTable->getTempNonTrackedVariable('variable', $compilationContext, true);
                                $expression = new Expression(array(
                                    'type' => 'static-property-access',
                                    'left' => array(
                                        'value' => $statement['variable']
                                    ),
                                    'right' => array(
                                        'value' => $statement['property']
                                    )
                                ));
                                $expression->setExpectReturn(true, $tempVariable);
                                $expression->compile($compilationContext);
                                $compilationContext->codePrinter->output('zephir_concat_function(' . $variableVariable->getPointeredName() . ', ' . $tempVariable->getPointeredName() . ', '.$variableVariable->getPointeredName().');');
                                //continue

                            case 'assign':
                                $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), &' . $variableVariable->getName() . ' TSRMLS_CC);');
                                if ($variableVariable->isTemporal()) {
                                    $variableVariable->setIdle(true);
                                }
                                break;
                            default:
                                throw new CompilerException("Operator '" . $statement['operator'] . "' is not supported for variable type: string", $statement);
                        }
                        break;
                    case 'variable':
                    case 'array':
                        $codePrinter->output('zephir_update_static_property_ce(' . $classEntry .', SL("' . $property . '"), ' . $variableVariable->getPointeredName() . ');');
                        if ($variableVariable->isTemporal()) {
                            $variableVariable->setIdle(true);
                        }
                        break;

                    default:
                        throw new CompilerException("Unknown type " . $variableVariable->getType(), $statement);
                }
                break;

            default:
                throw new CompilerException("Unknown type " . $resolvedExpr->getType(), $statement);
        }
    }
}
