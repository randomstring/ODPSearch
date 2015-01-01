/************************************************************************
 
 The contents of this file are subject to the Mozilla Public License
 Version 1.1 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 License for the specific language governing rights and limitations
 under the License.
 
 The Original Code is Isearch.
 
 The Initial Developer of the Original Code is Clearinghouse for
 Networked Information Discovery and Retrieval.
 
 Portions created by Netscape Communications are Copyright (C) 2001-2002.
 All Rights Reserved.
 
 Portions created by the Clearinghouse for Networked Information
 Discovery and Retrieval are Copyright (C) 1994. All Rights Reserved.
 
************************************************************************/

#define BUCKET_1_MAX     100
#define BUCKET_1_SIZE      5
#define BUCKET_2_MAX     700
#define BUCKET_2_SIZE     10
#define BUCKET_3_MAX    2000
#define BUCKET_3_SIZE     50
#define BUCKET_4_MAX   10000
#define BUCKET_4_SIZE    100
#define BUCKET_5_MAX  400000
#define BUCKET_5_SIZE   5000

unsigned char 
pricebucket(float price) {

    int p = (int) price;
    int bucket_start = 0;
    int bucket = 0;

    if (p <= 0) 
        return 0;

    if ((0 < price) && ( price <= BUCKET_1_MAX)) {
      bucket = p / BUCKET_1_SIZE;
    }
    else {
      bucket_start = BUCKET_1_MAX / BUCKET_1_SIZE;

      if ((BUCKET_1_MAX < price) && ( price <= BUCKET_2_MAX)) {
	bucket = bucket_start + ((p - BUCKET_1_MAX) / BUCKET_2_SIZE);
      }
      else {
	bucket_start += (BUCKET_2_MAX - BUCKET_1_MAX) / BUCKET_2_SIZE;

	if ((BUCKET_2_MAX < price) && ( price <= BUCKET_3_MAX)) {
	  bucket = bucket_start + ((p - BUCKET_2_MAX) / BUCKET_3_SIZE);
	}
	else {
	  bucket_start += (BUCKET_3_MAX - BUCKET_2_MAX) / BUCKET_3_SIZE;

	  if ((BUCKET_3_MAX < price) && ( price <= BUCKET_4_MAX)) {
	    bucket = bucket_start + ((p - BUCKET_3_MAX) / BUCKET_4_SIZE);
	  }
	  else {
	    bucket_start += (BUCKET_4_MAX - BUCKET_3_MAX) / BUCKET_4_SIZE;

	    if ((BUCKET_4_MAX < price) && ( price <= BUCKET_5_MAX)) {
	      bucket = bucket_start + ((p - BUCKET_4_MAX) / BUCKET_5_SIZE);
	    }
	    else {
	      bucket_start += (BUCKET_5_MAX - BUCKET_4_MAX) / BUCKET_5_SIZE;
	      bucket = 255;
	    }	      
	  }
	}
      }
    }

    if (bucket < 0)
      bucket = 0;
    
    if (bucket > 255) 
      bucket = 255;

    return bucket;

}

float
pricebucketrange(unsigned char bucket, int max) {

  float price = 0.0;

  if (max && (bucket == 255)) 
    return (100000.00);

  if (max) {
    bucket++;
  }

  if (bucket <= (BUCKET_1_MAX / BUCKET_1_SIZE)) {
    price = bucket * BUCKET_1_SIZE;
  }
  else {
    bucket -= (BUCKET_1_MAX / BUCKET_1_SIZE);
    price += BUCKET_1_MAX;
    if (bucket <= ((BUCKET_2_MAX - BUCKET_1_MAX)/ BUCKET_2_SIZE)) 
      price += bucket * BUCKET_2_SIZE;
    else {
      bucket -= ((BUCKET_2_MAX - BUCKET_1_MAX)/ BUCKET_2_SIZE);
      price = BUCKET_2_MAX;
      if (bucket <= ((BUCKET_3_MAX - BUCKET_2_MAX)/ BUCKET_3_SIZE)) 
	price += bucket * BUCKET_3_SIZE;
      else {
	bucket -= ((BUCKET_3_MAX - BUCKET_2_MAX)/ BUCKET_3_SIZE);
	price = BUCKET_3_MAX;
	if (bucket <= ((BUCKET_4_MAX - BUCKET_3_MAX)/ BUCKET_4_SIZE)) 
	  price += bucket * BUCKET_4_SIZE;
	else {
	  bucket -= ((BUCKET_4_MAX - BUCKET_3_MAX)/ BUCKET_4_SIZE);
	  price = BUCKET_4_MAX;
	  if (bucket <= ((BUCKET_5_MAX - BUCKET_4_MAX)/ BUCKET_5_SIZE)) 
	    price += bucket * BUCKET_5_SIZE;
	  else {
	    bucket -= ((BUCKET_5_MAX - BUCKET_4_MAX)/ BUCKET_5_SIZE);
	    price = BUCKET_5_MAX;
	}
	}
      }
    }
  }

  if (max)
    price -= 0.01;

  return price;

}

#if 0
int
main() {

  float f;
  unsigned char j,l;

  l = 255;
  for(f = 0.0; f < 500000.0; f += 0.50) {
    j = pricebucket(f);
    if (j != l) {
      printf("Price %9.2f  => %3d\n",f,j);
      l = j;
    }
  }
}

#endif
