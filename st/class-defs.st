"
Copyright (c) 2008 Vincent Geddes
Copyright (c) 2007 Luca Bruno

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the 'Software'), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
"

"Basic meta"

nil subclass: #Object
    instanceVariableNames: ''
    classVariableNames: ''!

Object subclass: #UndefinedObject
       instanceVariableNames: ''
       classVariableNames: ''!

Object subclass: #Behavior
       instanceVariableNames: 'superclass instanceSize methodDictionary instanceVariables'
       classVariableNames: ''!

Behavior subclass: #Class
         instanceVariableNames: 'name classPool'
	     classVariableNames: ''!

Behavior subclass: #Metaclass
	instanceVariableNames: 'instanceClass'
	classVariableNames: ''!


"Collections"

Object subclass: #Collection
       instanceVariableNames: ''
       classVariableNames: ''!

Collection subclass: #HashedCollection
           instanceVariableNames: 'tally array'
           classVariableNames: ''!

HashedCollection subclass: #Set
		 instanceVariableNames: ''
		 classVariableNames: ''!

HashedCollection subclass: #Dictionary
		 instanceVariableNames: ''
		 classVariableNames: ''!

Collection subclass: #SequenceableCollection
	   instanceVariableNames: ''
           classVariableNames: ''!

SequenceableCollection subclass: #Interval
	   instanceVariableNames: 'start stop step'
	   classVariableNames: ''!

SequenceableCollection subclass: #ArrayedCollection
		       instanceVariableNames: ''
                       classVariableNames: ''!

ArrayedCollection subclass: #Array
		  instanceVariableNames: ''
                  classVariableNames: ''!

ArrayedCollection subclass: #ByteArray
		  instanceVariableNames: ''
                  classVariableNames: ''!

SequenceableCollection subclass: #OrderedCollection
		       instanceVariableNames: 'tally array'
                       classVariableNames: ''!

"Execution"

Object subclass: #ContextPart
       instanceVariableNames: 'sender method ip sp'
       classVariableNames: ''!

ContextPart subclass: #MethodContext
	        instanceVariableNames: 'receiver'
            classVariableNames: ''!

ContextPart subclass: #BlockContext
	        instanceVariableNames: 'home'
            classVariableNames: ''!

Object subclass: #Process
       instanceVariableNames: 'context suspended returnedObject next scheduled'
       classVariableNames: ''!

Object subclass: #CompiledCode
       instanceVariableNames: 'header bytecode literals'
       classVariableNames: ''!

CompiledCode subclass: #CompiledMethod
	     instanceVariableNames: ''
	     classVariableNames: ''!

CompiledCode subclass: #CompiledBlock
	     instanceVariableNames: 'method'
	     classVariableNames: ''!

Object subclass: #Message
	   instanceVariableNames: 'selector arguments'
	   classVariableNames: ''!


"Streams"

Object subclass: #OutputStream
	   instanceVariableNames: ''
	   classVariableNames: ''!

"Basic data types"

ByteArray subclass: #String
	  instanceVariableNames: ''
          classVariableNames: ''!

String subclass: #Symbol
       instanceVariableNames: ''
       classVariableNames: ''!

Object subclass: #Magnitude
       instanceVariableNames: ''
       classVariableNames: ''!

Magnitude subclass: #Character
	  instanceVariableNames: 'value'
          classVariableNames: 'WhiteSpaceTable XDigitsTable'!

Magnitude subclass: #Number
	  instanceVariableNames: ''
          classVariableNames: ''!


Number subclass: #Integer
       instanceVariableNames: ''
       classVariableNames: ''!

Integer subclass: #SmallInteger
	instanceVariableNames: ''
	classVariableNames: ''!

Integer subclass: #LargeInteger
	instanceVariableNames: ''
	classVariableNames: ''!

Number subclass: #Fraction
       instanceVariableNames: 'numerator denominator'
       classVariableNames: ''!

Number subclass: #Float
       instanceVariableNames: ''
       classVariableNames: ''!

Object subclass: #Boolean
       instanceVariableNames: ''
       classVariableNames: ''!

Boolean subclass: #True
	instanceVariableNames: ''
        classVariableNames: ''!

Boolean subclass: #False
	instanceVariableNames: ''
        classVariableNames: ''!

"Dictionaries"

Object subclass: #Association
       instanceVariableNames: 'key value'
       classVariableNames: ''!


Dictionary subclass: #IdentityDictionary
	   instanceVariableNames: ''
	   classVariableNames: ''!

IdentityDictionary subclass: #SystemDictionary
		   instanceVariableNames: 'systemInitializationRequests commandLineArguments startupProcess startupObject continueStartup'
		   classVariableNames: ''!

