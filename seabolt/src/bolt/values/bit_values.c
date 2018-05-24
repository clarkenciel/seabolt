/*
 * Copyright (c) 2002-2018 "Neo Technology,"
 * Network Engine for Objects in Lund AB [http://neotechnology.com]
 *
 * This file is part of Neo4j.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <string.h>
#include <bolt/values.h>
#include <assert.h>



void BoltValue_format_as_Boolean(struct BoltValue * value, char data)
{
    _format(value, BOLT_BOOLEAN, 0, 1, NULL, 0);
    value->data.as_char[0] = data;
}

char BoltBoolean_get(const struct BoltValue * value)
{
    return to_bit(value->data.as_char[0]);
}

int BoltBoolean_write(const struct BoltValue * value, FILE * file)
{
    assert(BoltValue_type(value) == BOLT_BOOLEAN);
    fprintf(file, "!%d", BoltBoolean_get(value));
    return 0;
}


void BoltValue_format_as_Bytes(struct BoltValue * value, char * data, int32_t length)
{
    if (length <= sizeof(value->data) / sizeof(char))
    {
        _format(value, BOLT_BYTES, 0, length, NULL, 0);
        if (data != NULL)
        {
            memcpy(value->data.as_char, data, (size_t)(length));
        }
    }
    else
    {
        _format(value, BOLT_BYTES, 0, length, data, sizeof_n(char, length));
    }
}

char BoltBytes_get(const struct BoltValue * value, int32_t index)
{
    const char* data = value->size <= sizeof(value->data) / sizeof(char) ?
                       value->data.as_char : value->data.extended.as_char;
    return data[index];
}

char* BoltBytes_get_all(struct BoltValue * value)
{
    return value->size <= sizeof(value->data) / sizeof(char) ?
           value->data.as_char : value->data.extended.as_char;
}

int BoltBytes_write(const struct BoltValue * value, FILE * file)
{
    assert(BoltValue_type(value) == BOLT_BYTES);
    fprintf(file, "#[");
    for (int i = 0; i < value->size; i++)
    {
        char b = BoltBytes_get(value, i);
        fprintf(file, "%c%c", hex1(&b, 0), hex0(&b, 0));
    }
    fprintf(file, "]");
    return 0;
}
