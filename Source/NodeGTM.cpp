/*=========================================================================
 *
 *  Copyright OSEHRA
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "NodeGTM.h"

#include <cstring>

#define THROW_EXCEPTION(message) \
v8::ThrowException(v8::Exception::TypeError(v8::String::New(message))); \
    return scope.Close(v8::Undefined());

#define FUNCTION_PROTOTYPE(nodeName,cppName) \
  tpl->PrototypeTemplate()->Set(v8::String::NewSymbol(nodeName), \
      v8::FunctionTemplate::New(cppName)->GetFunction());


//
//  Static function to initialize object.
//
void NodeGTM::Initialize( v8::Handle<v8::Object> target)
{
 // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(New);

  tpl->SetClassName(v8::String::NewSymbol("Database"));

  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  FUNCTION_PROTOTYPE("get",Get);
  FUNCTION_PROTOTYPE("set",Set);
  FUNCTION_PROTOTYPE("kill",Kill);
  FUNCTION_PROTOTYPE("order",Order);
  FUNCTION_PROTOTYPE("query",Query);

  v8::Persistent<v8::Function> constructor =
      v8::Persistent<v8::Function>::New( tpl->GetFunction() );

  target->Set(v8::String::NewSymbol("Database"), constructor);
}

//
//  Static Method to create a New object
//
v8::Handle<v8::Value> NodeGTM::New(const v8::Arguments& args)
{
  v8::HandleScope scope;

  NodeGTM * obj = new NodeGTM();

  obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();

  obj->Wrap(args.This());

  return args.This();
}


//
// Constructor
//
NodeGTM::NodeGTM()
{
  // Initialize GTM
  CALLGTM( gtm_init() );

  this->counter_ = 0.0;
}


//
// Destructor
//
NodeGTM::~NodeGTM()
{
  // Cleanup GT.M runtime
  CALLGTM( gtm_exit() );
}


//
//  Get the value of a Global from GT.M
//
v8::Handle<v8::Value> NodeGTM::Get(const v8::Arguments& args)
{
  v8::HandleScope scope;

  NodeGTM * obj = ObjectWrap::Unwrap<NodeGTM >( args.This() );

  gtm_char_t valueOfGlobal[maxValueLength];
  gtm_char_t nameOfGlobal[maxValueLength];
  gtm_char_t errorMessage[maxMessageLength];

  if( args[0]->IsString() )
    {
    v8::Local<v8::String> name = args[0]->ToString();
    name->WriteAscii( nameOfGlobal );
    }
  else
    {
    THROW_EXCEPTION("Argument was not a String");
    }

  //
  // Now we delegate the task to the GT.M interface
  //
  obj->Get( nameOfGlobal, valueOfGlobal, errorMessage );

  if ( strlen( errorMessage ) != 0 )
    {
    THROW_EXCEPTION( errorMessage );
    }

  return scope.Close(v8::String::New( valueOfGlobal ));
}


//
//  Get the value of a Global from GT.M
//
void NodeGTM::Get( const gtm_char_t * nameOfGlobal, gtm_char_t * valueOfGlobal, gtm_char_t * errorMessage )
{
  std::cout << "calling Get( " << nameOfGlobal << " ) " << std::endl;

  gtm_string_t p_value;

  p_value.address = ( xc_char_t *) &valueOfGlobal;
  p_value.length = maxValueLength ;


  CALLGTM( gtm_ci( "gtmget", nameOfGlobal, &p_value, &errorMessage ));
}


//
//  Set the value of a Global from GT.M
//
v8::Handle<v8::Value> NodeGTM::Set(const v8::Arguments& args)
{
  v8::HandleScope scope;

  NodeGTM * obj = ObjectWrap::Unwrap<NodeGTM >( args.This() );

  gtm_char_t valueOfGlobal[maxValueLength];
  gtm_char_t nameOfGlobal[maxValueLength];
  gtm_char_t errorMessage[maxValueLength];

  if( args.Length() < 2 )
    {
    THROW_EXCEPTION("Wrong number of arguments. Expected two");
    }

  if( args[0]->IsString() )
    {
    v8::Local<v8::String> name = args[0]->ToString();
    name->WriteAscii( nameOfGlobal );
    }
  else
    {
    THROW_EXCEPTION("First argument was not a String");
    }


  if( args[1]->IsString() )
    {
    v8::Local<v8::String> name = args[1]->ToString();
    name->WriteAscii( valueOfGlobal );
    }
  else
    {
    THROW_EXCEPTION("Second argument was not a String");
    }

  //
  // Now we delegate the task to the GT.M interface
  //
  obj->Set( nameOfGlobal, valueOfGlobal, errorMessage );

  if ( strlen( errorMessage ) != 0 )
    {
    THROW_EXCEPTION( errorMessage );
    }

  //
  // TODO: Discuss with community if this return is a good idea...
  //
  return scope.Close( args[1]->ToString() );
}


//
//  Get the value of a Global from GT.M
//
void NodeGTM::Set( const gtm_char_t * nameOfGlobal, const gtm_char_t * valueOfGlobal, gtm_char_t * errorMessage )
{
  std::cout << "calling Set( " << nameOfGlobal << " ) = " << valueOfGlobal << std::endl;

  gtm_string_t p_value;

  p_value.address = ( xc_char_t *) &valueOfGlobal;
  p_value.length = strlen( valueOfGlobal );


  CALLGTM( gtm_ci( "gtmset", nameOfGlobal, &p_value, &errorMessage ));
}


//
//  Kill (delete) a Global through GT.M
//
v8::Handle<v8::Value> NodeGTM::Kill(const v8::Arguments& args)
{
  v8::HandleScope scope;

  NodeGTM * obj = ObjectWrap::Unwrap<NodeGTM >( args.This() );

  gtm_char_t nameOfGlobal[maxValueLength];
  gtm_char_t errorMessage[maxValueLength];

  if( args.Length() != 1 )
    {
    THROW_EXCEPTION("Wrong number of arguments. Expected one");
    }

  if( args[0]->IsString() )
    {
    v8::Local<v8::String> name = args[0]->ToString();
    name->WriteAscii( nameOfGlobal );
    }
  else
    {
    THROW_EXCEPTION("First argument was not a String");
    }

  //
  // Now we delegate the task to the GT.M interface
  //
  obj->Kill( nameOfGlobal, errorMessage );

  if ( strlen( errorMessage ) != 0 )
    {
    THROW_EXCEPTION( errorMessage );
    }

  //
  // TODO: Discuss with community if this return is a good idea...
  //
  return scope.Close( args[0]->ToString() );
}


//
//  Kill a Global in GT.M
//
void NodeGTM::Kill( const gtm_char_t * nameOfGlobal, gtm_char_t * errorMessage )
{
  std::cout << "calling Kill( " << nameOfGlobal << " ) " << std::endl;

  CALLGTM( gtm_ci( "gtmkill", nameOfGlobal, &errorMessage ));
}


//
//  Get the value of the next index in a Global from GT.M
//
v8::Handle<v8::Value> NodeGTM::Order(const v8::Arguments& args)
{
  v8::HandleScope scope;

  NodeGTM * obj = ObjectWrap::Unwrap<NodeGTM >( args.This() );

  gtm_char_t valueOfIndex[maxValueLength];
  gtm_char_t nameOfGlobal[maxValueLength];
  gtm_char_t errorMessage[maxMessageLength];

  if( args[0]->IsString() )
    {
    v8::Local<v8::String> name = args[0]->ToString();
    name->WriteAscii( nameOfGlobal );
    }
  else
    {
    THROW_EXCEPTION("Argument was not a String");
    }

  //
  // Now we delegate the task to the GT.M interface
  //
  obj->Order( nameOfGlobal, valueOfIndex, errorMessage );

  if ( strlen( errorMessage ) != 0 )
    {
    THROW_EXCEPTION( errorMessage );
    }

  return scope.Close(v8::String::New( valueOfIndex ));
}


//
//  Get the value of the next index in a Global from GT.M
//
void NodeGTM::Order( const gtm_char_t * nameOfGlobal, gtm_char_t * valueOfIndex, gtm_char_t * errorMessage )
{
  std::cout << "calling Order( " << nameOfGlobal << " ) " << std::endl;

  gtm_string_t p_value;

  p_value.address = ( xc_char_t *) &valueOfIndex;
  p_value.length = maxValueLength ;


  CALLGTM( gtm_ci( "gtmorder", nameOfGlobal, &p_value, &errorMessage ));
}


//
//  Get the value of the next index in a Global from GT.M
//
v8::Handle<v8::Value> NodeGTM::Query(const v8::Arguments& args)
{
  v8::HandleScope scope;

  NodeGTM * obj = ObjectWrap::Unwrap<NodeGTM >( args.This() );

  gtm_char_t valueOfIndex[maxValueLength];
  gtm_char_t nameOfGlobal[maxValueLength];
  gtm_char_t errorMessage[maxMessageLength];

  if( args[0]->IsString() )
    {
    v8::Local<v8::String> name = args[0]->ToString();
    name->WriteAscii( nameOfGlobal );
    }
  else
    {
    THROW_EXCEPTION("Argument was not a String");
    }

  //
  // Now we delegate the task to the GT.M interface
  //
  obj->Query( nameOfGlobal, valueOfIndex, errorMessage );

  if ( strlen( errorMessage ) != 0 )
    {
    THROW_EXCEPTION( errorMessage );
    }

  return scope.Close(v8::String::New( valueOfIndex ));
}


//
//  Get the value of the next index in a Global from GT.M
//
void NodeGTM::Query( const gtm_char_t * nameOfGlobal, gtm_char_t * valueOfIndex, gtm_char_t * errorMessage )
{
  std::cout << "calling Query( " << nameOfGlobal << " ) " << std::endl;

  gtm_string_t p_value;

  p_value.address = ( xc_char_t *) &valueOfIndex;
  p_value.length = maxValueLength ;


  CALLGTM( gtm_ci( "gtmquery", nameOfGlobal, &p_value, &errorMessage ));
}


