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
 * ObjectDynamicProperty
 *
 * Updates object properties dynamically
 */
class ObjectDynamicStringProperty
{
    /**
     * Compiles foo->{"x"} = {expr}
     *
     * @param string $variable
     * @param Variable $symbolVariable
     * @param CompiledExpression $resolvedExpr
     * @param CompilationContext $compilationContext
     * @param array $statement
     */
    public function assign($variable, ZephirVariable $symbolVariable, CompiledExpression $resolvedExpr, CompilationContext $compilationContext, $statement)
    {
        if (!$symbolVariable->isInitialized()) {
            throw new CompilerException("Cannot mutate variable '" . $variable . "' because it is not initialized", $statement);
        }

        if ($symbolVariable->getType() != 'variable') {
            throw new CompilerException("Variable type '" . $symbolVariable->getType() . "' cannot be used as object", $statement);
        }

        $propertyName = $statement['property'];

        if (!is_string($propertyName)) {
            throw new CompilerException("Expected string to update object property, ".gettype($propertyName)." received", $statement);
        }

        if (!$symbolVariable->isInitialized()) {
            throw new CompilerException("Cannot mutate static property '" . $compilationContext->classDefinition->getCompleteName() . "::" . $propertyName . "' because it is not initialized", $statement);
        }

        if ($symbolVariable->getType() != 'variable') {
            throw new CompilerException("Cannot use variable type: " . $symbolVariable->getType() . " as an object", $statement);
        }

        if ($symbolVariable->hasAnyDynamicType('unknown')) {
            throw new CompilerException("Cannot use non-initialized variable as an object", $statement);
        }

        /**
         * Trying to use a non-object dynamic variable as object
         */
        if ($symbolVariable->hasDifferentDynamicType(array('undefined', 'object', 'null'))) {
            $compilationContext->logger->warning('Possible attempt to update property on non-object dynamic property', 'non-valid-objectupdate', $statement);
        }

        $codePrinter = $compilationContext->codePrinter;
        $compilationContext->headersManager->add('kernel/object');

        $propertyVariableName = $propertyName;

        switch ($resolvedExpr->getType()) {

            case 'null':
                $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                $codePrinter->output('ZVAL_NULL(&' . $tempVariable->getName() . ');');
                if ($variable == 'this') {
                    $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"), ' . $tempVariable->getPointeredName() . ');');
                } else {
                    $codePrinter->output('zephir_update_property_zval(&' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"), ' . $tempVariable->getPointeredName() . ');');
                }
                break;

            case 'int':
            case 'long':
                $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                $codePrinter->output('ZVAL_LONG(' . $tempVariable->getName() . ', ' . $resolvedExpr->getBooleanCode() . ');');
                if ($variable == 'this') {
                    $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                } else {
                    $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                }
                break;

            case 'string':
                $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                $codePrinter->output('ZVAL_STRING(&' . $tempVariable->getName() . ', "' . $resolvedExpr->getCode() . '");');
                if ($variable == 'this') {
                    $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                } else {
                    $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                }
                break;

            case 'bool':
                $boolVal = $resolvedExpr->getBooleanCode() == '1';
                if ($boolVal != '1' && $boolVal != '0') throw new Exception("Unknown Boolean Value" . $boolVal);
                $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                $codePrinter->output('ZVAL_BOOL(&' . $tempVariable->getName() . ', "' . $boolVal . '");');
                if ($variable == 'this') {
                    $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"), ' . $tempVariable->getPointeredName() . ');');
                } else {
                    $codePrinter->output('zephir_update_property_zval(&' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"), ' . $tempVariable->getPointeredName() . ');');
                }
                break;

            case 'empty-array':
                $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                $codePrinter->output('array_init(' . $tempVariable->getName() . ');');
                if ($variable == 'this') {
                    $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                } else {
                    $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                }
                break;

            case 'array':
                $variableVariable = $compilationContext->symbolTable->getVariableForRead($resolvedExpr->getCode(), $compilationContext, $statement);
                if ($variable == 'this') {
                    $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"),  ' . $variableVariable->getPointeredName() . ');');
                } else {
                    $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"),  ' . $variableVariable->getPointeredName() . ');');
                }
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
                        $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                        $codePrinter->output('ZVAL_LONG(' . $tempVariable->getName() . ', ' . $variableVariable->getName() . ');');
                        $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                        break;

                    case 'bool':
                        $tempVariable = $compilationContext->symbolTable->getTempVariableForWrite('variable', $compilationContext);
                        $codePrinter->output('ZVAL_BOOL(' . $tempVariable->getName() . ', ' . $variableVariable->getName() . ');');
                        if ($variable == 'this') {
                            $codePrinter->output('zephir_update_property_zval(this_ptr, SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                        } else {
                            $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyVariableName . '"),  ' . $tempVariable->getPointeredName() . ');');
                        }
                        break;

                    case 'string':
                    case 'variable':
                    case 'array':
                        $codePrinter->output('zephir_update_property_zval(' . $symbolVariable->getName() . ', SL("' . $propertyName . '"), &' . $resolvedExpr->getCode() . ');');
                        if ($symbolVariable->isTemporal()) {
                            $symbolVariable->setIdle(true);
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
