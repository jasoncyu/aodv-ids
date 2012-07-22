/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include <iostream>
// #include <string>

using namespace ns3;

class MyObject : public Object
{
public:
  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("MyObject")
      .SetParent (Object::GetTypeId ())
      .AddConstructor<MyObject> ()
      .AddTraceSource ("MyInteger",
                       "An integer value to trace.",
                       MakeTraceSourceAccessor (&MyObject::m_myInt))
      .AddTraceSource ("MyOtherInteger",
                       "Another integer value to trace.",
                       MakeTraceSourceAccessor (&MyObject::m_myOtherInt))
      // .AddTraceSource ("MyString",
                       // "A string to trace.",
                       // MakeTraceSourceAccessor (&MyObject::m_myString))
    ;

    return tid;
  }

  MyObject () {}
  TracedValue<int32_t> m_myInt;
  TracedValue<int32_t> m_myOtherInt;
  // TracedValue<string> m_myString;
};

void
IntTrace (int32_t oldValue, int32_t newValue)
{
  std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}

// void
// StringTrace (string oldValue, string newValue)
// {
//   std::cout << "String Traced " << oldValue << " to " << newValue << std::endl;
// }

int
main (int argc, char *argv[])
{
  Ptr<MyObject> myObject = CreateObject<MyObject> ();
  myObject->TraceConnectWithoutContext ("MyInteger", MakeCallback (&IntTrace));
  // myObject->TraceConnectWithoutContext ("MyString", MakeCallback (&StringTrace));
  myObject->TraceConnectWithoutContext ("MyOtherInteger", MakeCallback (&IntTrace));

  myObject->m_myInt = 1234;
  myObject->m_myOtherInt = 9876;
  // myObject->m_myString = "hello ns3";
}
