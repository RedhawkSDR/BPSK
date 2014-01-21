/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef BPSK_IMPL_H
#define BPSK_IMPL_H

#include "BPSK_base.h"

class BPSK_i;

class BPSK_i : public BPSK_base
{
    ENABLE_LOGGING
    public: 
        BPSK_i(const char *uuid, const char *label);
        ~BPSK_i();
        int serviceFunction();
    private:
        int m_zeroCrossing;
        int m_signLast;
        float m_symbolIntegrator;
        double m_currXdelta;
        double m_outputRate;
        bool m_propChanged;

        void propertyChangeListener(const std::string&);
        void checkProperties();
};

#endif
