#include <quiz.h>

#include "execution_environment.h"

QUIZ_CASE(python_numpy) {
  TestExecutionEnvironment env = init_environement();
  // Test "from matplotlib.pyplot import *"
  assert_command_execution_fails(env, "np.array([1,3,4])");
  assert_command_execution_succeeds(env, "from numpy import *");
  // TODO: Remove the "dtype="
  assert_command_execution_succeeds(env, "ones(3)",
                                    "array([1.0, 1.0, 1.0], dtype=float64)\n");

  deinit_environment();
  env = init_environement();
  assert_command_execution_succeeds(env, "import numpy as np");
  assert_command_execution_succeeds(env, "arr1 = np.array([1,3,5,4])");
  assert_command_execution_succeeds(
      env, "np.cos(arr1)",
      "array([0.5403023058681398, -0.9899924966004454, 0.2836621854632262, "
      "-0.6536436208636119], dtype=float64)\n");
  assert_command_execution_succeeds(env, "arr2 = np.arange(1,5)");
  assert_command_execution_succeeds(env, "np.linspace(1,10,2)",
                                    "array([1.0, 10.0], dtype=float64)\n");
  assert_command_execution_succeeds(
      env, "np.ones(4)", "array([1.0, 1.0, 1.0, 1.0], dtype=float64)\n");
  assert_command_execution_succeeds(env, "np.zeros(2)",
                                    "array([0.0, 0.0], dtype=float64)\n");
  assert_command_execution_succeeds(
      env, "arr1.flatten()", "array([1.0, 3.0, 5.0, 4.0], dtype=float64)\n");
  assert_command_execution_succeeds(
      env, "arr1.reshape((2,2))",
      "array([[1.0, 3.0],\n       [5.0, 4.0]], dtype=float64)\n");
  assert_command_execution_succeeds(env, "arr1.shape", "(4,)\n");
  assert_command_execution_succeeds(env, "arr1.tolist()",
                                    "[1.0, 3.0, 5.0, 4.0]\n");
  assert_command_execution_succeeds(
      env, "arr1.transpose()", "array([1.0, 3.0, 5.0, 4.0], dtype=float64)\n");
  assert_command_execution_succeeds(env, "np.argmax(arr1)", "2\n");
  assert_command_execution_succeeds(env, "np.argmin(arr1)", "0\n");
  assert_command_execution_succeeds(env, "np.dot(arr1,arr2)", "38.0\n");
  assert_command_execution_succeeds(env, "np.max(arr1)", "5.0\n");
  assert_command_execution_succeeds(env, "np.min(arr1)", "1.0\n");
  assert_command_execution_succeeds(env, "np.mean(arr1)", "3.25\n");
  assert_command_execution_succeeds(env, "np.median(arr1)", "3.5\n");
  assert_command_execution_succeeds(
      env, "np.polyfit(arr1,arr2,3)",
      "array([-0.4999999999997726, 4.499999999998181, -10.99999999999454, "
      "7.999999999995907], dtype=float64)\n");
  assert_command_execution_succeeds(env, "np.polyval([1,2,3],[1])",
                                    "array([6.0], dtype=float64)\n");
  assert_command_execution_succeeds(env, "np.size(arr1)", "4\n");
  assert_command_execution_succeeds(
      env, "np.sort(arr1)", "array([1.0, 3.0, 4.0, 5.0], dtype=float64)\n");
  assert_command_execution_succeeds(env, "np.std(arr1)", "1.479019945774904\n");
  assert_command_execution_succeeds(env, "np.sum(arr1)", "13.0\n");
  assert_command_execution_succeeds(env, "a = np.array([[1, 2], [3, 4]])");
  assert_command_execution_succeeds(env, "b = np.array([[5, 6]])");
  assert_command_execution_succeeds(env, "np.concatenate((a,b))",
                                    "array([[1.0, 2.0],\n       [3.0, 4.0],\n  "
                                    "     [5.0, 6.0]], dtype=float64)\n");
  // TODO: output should be "array([-2, 4, -2], dtype=float64)"
  assert_command_execution_succeeds(
      env, "np.cross(np.array([1,2,3]),np.array([3,4,5]))",
      "array([-2.0, 4.0, -2.0], dtype=float64)\n");
}
