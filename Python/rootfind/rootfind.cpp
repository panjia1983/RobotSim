#include "function.h"
#include "Newton.h"
#include "root.h"
#include "pyvectorfield.h"
#include "rootfind.h"
#include "pyerr.h"

#include <Python.h>
#include <iostream>
#include <exception>

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

using std::cout;
using std::endl;

using std::exception;

using namespace PyPlanner;
	
using Math::Vector;
using Math::ConvergenceResult;
using Optimization::NewtonRoot;

double rootTolF = 1e-4;
double rootTolX = 1e-7;
NewtonRoot* root = NULL;
PyVectorFieldFunction* theFn = NULL;

// TODO: Python exceptions

void setFTolerance(double tolf)
{
  rootTolF = tolf;
  if(root) root->tolf=tolf;
}

void setXTolerance(double tolx)
{
  rootTolX = tolx;
  if(root) root->tolx=tolx;
}

// Instance of subclass of Python VectorFieldFunction object 
// (see vectorfield.py) - pVFObj
int setVectorField(PyObject* pVFObj) {
	destroy();
	
	theFn = new PyVectorFieldFunction(pVFObj);
	if(!theFn->is_init()) {
		delete theFn;
		theFn = NULL;
		return 0;
	}
	
	root = new NewtonRoot(theFn);
	root->tolf = rootTolF;
	root->tolx = rootTolX;
	return 1;
}

PyObject* PyListFromVector(const Vector& x) {
	// HACK: gcc doesn't recognize the existence of x.size() (?!)
	PyObject* ls = PyList_New(x.n);
	PyObject* pItem;
	if(ls == NULL) {
		goto fail;
	}
	
	for(Py_ssize_t i = 0; i < PySequence_Size(ls); i++) {
		pItem = PyFloat_FromDouble(x[(int)i]);
		if(pItem == NULL)
			goto fail;
		PyList_SetItem(ls, i, pItem);
	}
	
	return ls;
	
	fail:
		Py_XDECREF(ls);
		return NULL;
}

PyObject* findRoots(PyObject* startVals, int iter) {
	if(root == NULL) {
	  throw PyException("rootfind.findRoots: no vector field set");
	  return NULL;
	}
	
	if(!PySequence_Check(startVals)) {
	  throw PyException("rootfind.findRoots: starting value is not a sequence");
	  return NULL;
	}
	if(PySequence_Size(startVals) != theFn->NumVariables()) {
	  throw PyException("rootfind.findRoots: starting value has incorrect size");
	}
	
	root->x.resize(theFn->NumVariables());
	for(Py_ssize_t i = 0; i < PySequence_Size(startVals); i++) {
		(root->x)[(int)i] = PyFloat_AsDouble(PySequence_GetItem(startVals, i));
	}
	
	ConvergenceResult conv = root->Solve(iter);
	
	//HACK
	PyObject* tuple,*ls,*code,*usedIters;
	ls = PyListFromVector(root->x);
	if(ls == NULL) {
	  throw PyException("rootfind.findRoots: unable to allocate return value");
	}
	tuple = PyTuple_New(3);
	if(tuple == NULL) {
	  Py_XDECREF(ls);
	  throw PyException("rootfind.findRoots: unable to allocate return value");
	  return NULL;
	}
	switch(conv) {
		case Math::ConvergenceX:
			code = PyInt_FromLong(0);
			break;
		case Math::ConvergenceF:
			code = PyInt_FromLong(1);
			break;
		case Math::Divergence:
			code = PyInt_FromLong(2);
			break;
		case Math::LocalMinimum:
			code = PyInt_FromLong(3);
			break;
		case Math::MaxItersReached:
			code = PyInt_FromLong(4);
			break;
		case Math::ConvergenceError:
		default:
			code = PyInt_FromLong(5);
			break;
	}
	usedIters = PyInt_FromLong(iter);
	PyTuple_SetItem(tuple, 0, code);
	PyTuple_SetItem(tuple, 1, ls);
	PyTuple_SetItem(tuple, 2, usedIters);
	return tuple;
}

PyObject* findRootsBounded(PyObject* startVals, PyObject* boundVals, int iter) {
	if(root == NULL) {
	  throw PyException("rootfind.findRootsBounded: no vector field set");
	  return NULL;
	}
	
	if(!PySequence_Check(boundVals)) {
	  throw PyException("rootfind.findRootsBounded: bounds are not a sequence");
	  return NULL;
	}
	if(PySequence_Size(boundVals) != theFn->NumVariables()) {
	  throw PyException("rootfind.findRootsBounded: bounds have incorrect size");
	  return NULL;
	}
	
	root->bmin.resize(theFn->NumVariables());
	root->bmax.resize(theFn->NumVariables());
	for(int i = 0; i < root->bmin.n; i++) {
	  PyObject* tup = PySequence_GetItem(boundVals, (Py_ssize_t)i);
	  if(!PySequence_Check(tup) || PySequence_Size(tup) != (Py_ssize_t)2) {
	    throw PyException("rootfind.findRootsBounded: bound element is not a pair");
	    return NULL;
	  }
	  root->bmin[i] = PyFloat_AsDouble(PySequence_GetItem(tup, 0));
	  root->bmax[i] = PyFloat_AsDouble(PySequence_GetItem(tup, 1));
	}
	//cout<<"Parsed bounds:"<<endl;
	//cout<<root->bmin<<endl;
	//cout<<root->bmax<<endl;
	//getchar();
	return findRoots(startVals, iter);
}

void destroy() {
	if(theFn != NULL) {
		delete theFn;
	}
	if(root != NULL) {
		delete root;
	}
	theFn = NULL;
	root = NULL;
}

