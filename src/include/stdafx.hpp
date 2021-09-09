#ifndef _STDAFX_HPP
#define _STDAFX_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include "bswap.hpp"
#include "types.hpp"

// CryptoPP
//#include "cryptlib.h"
//using CryptoPP::Exception;
#include "hmac.h"
using CryptoPP::HMAC;
#include "sha.h"
using CryptoPP::SHA1;
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "arc4.h"
using CryptoPP::Weak::ARC4;

#include "globals.hpp"
#include "utils.hpp"
#include "crypto.hpp"
#include "images.hpp"

using namespace std;
#endif