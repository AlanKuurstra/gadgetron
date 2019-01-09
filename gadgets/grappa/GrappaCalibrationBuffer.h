#ifndef GRAPPACALIBRATIONBUFFER_H
#define GRAPPACALIBRATIONBUFFER_H

#include "gadgetron_grappa_export.h"
#include "ismrmrd/ismrmrd.h"
#include "hoNDArray.h"
#include "GrappaWeights.h"
#include "GrappaWeightsCalculator.h"

#include <vector>
#include <string.h>
#include <memory>
#include <complex>

namespace Gadgetron {

    class EXPORTGADGETSGRAPPA CalibrationBufferCounter {

    public:
        CalibrationBufferCounter(unsigned int lines) {
            lines_sampled_ = std::vector<unsigned int>(lines, 0);
            memset(position_, 0, 3 * sizeof(float));
            memset(read_dir_, 0, 3 * sizeof(float));
            memset(phase_dir_, 0, 3 * sizeof(float));
            memset(slice_dir_, 0, 3 * sizeof(float));
        }


        virtual ~CalibrationBufferCounter() {}

        int update_line(unsigned int ky_index, float *position,
                        float *read_dir, float *phase_dir, float *slice_dir) {
            int ret_val = 0;

            if (!read_dir_equal(read_dir) ||
                !phase_dir_equal(phase_dir) ||
                !slice_dir_equal(slice_dir) ||
                !position_equal(position)) {
                for (unsigned int i = 0; i < lines_sampled_.size(); i++) {
                    lines_sampled_[i] = 0;
                }
                memcpy(position_, position, 3 * sizeof(float));
                memcpy(read_dir_, read_dir, 3 * sizeof(float));
                memcpy(phase_dir_, phase_dir, 3 * sizeof(float));
                memcpy(slice_dir_, slice_dir, 3 * sizeof(float));
                ret_val = 1;
            }

            if (ky_index >= lines_sampled_.size()) {
                return -1;
            }

            lines_sampled_[ky_index] = 1;

            return ret_val;
        }

        int get_region_of_support(unsigned int &min_ky_index, unsigned int &max_ky_index) {

            unsigned int current_start_line = 0;
            min_ky_index = 0;
            max_ky_index = 0;
            while (current_start_line < lines_sampled_.size()) {
                while ((current_start_line < lines_sampled_.size()) && (lines_sampled_[current_start_line] == 0)) {
                    current_start_line++;
                }
                if (current_start_line >= lines_sampled_.size()) continue;

                unsigned int region_start = current_start_line;
                while ((current_start_line < lines_sampled_.size()) && (lines_sampled_[current_start_line] > 0)) {
                    current_start_line++;
                }
                unsigned int region_end = current_start_line - 1;
                if ((region_start < region_end) && ((region_end - region_start) > (max_ky_index - min_ky_index))) {
                    min_ky_index = region_start;
                    max_ky_index = region_end;
                }
            }
            return 0;
        }

    protected:
        float position_[3];
        float read_dir_[3];
        float phase_dir_[3];
        float slice_dir_[3];

        bool position_equal(float *position) {
            for (unsigned int i = 0; i < 3; i++) {
                if (position_[i] != position[i]) return false;
            }
            return true;
        }

        bool read_dir_equal(float *cosines) {
            for (unsigned int i = 0; i < 3; i++) {
                if (read_dir_[i] != cosines[i]) return false;
            }
            return true;
        }

        bool phase_dir_equal(float *cosines) {
            for (unsigned int i = 0; i < 3; i++) {
                if (phase_dir_[i] != cosines[i]) return false;
            }
            return true;
        }

        bool slice_dir_equal(float *cosines) {
            for (unsigned int i = 0; i < 3; i++) {
                if (slice_dir_[i] != cosines[i]) return false;
            }
            return true;
        }

    private:
        std::vector<unsigned int> lines_sampled_;

    };

    class EXPORTGADGETSGRAPPA GrappaCalibrationBuffer {

    public:
        GrappaCalibrationBuffer(std::vector<size_t> dimensions,
                                boost::shared_ptr<GrappaWeights<float> > w,
                                GrappaWeightsCalculator<float> *weights_calculator);

        virtual ~GrappaCalibrationBuffer() {}

        int add_data(ISMRMRD::AcquisitionHeader *m1, hoNDArray<std::complex<float> > *m2,
                     unsigned short line_offset = 0, unsigned short partition_offset = 0);

    private:
        hoNDArray<std::complex<float> > buffer_;
        std::vector<size_t> dimensions_;
        boost::shared_ptr<GrappaWeights<float> > weights_;
        GrappaWeightsCalculator<float> *weights_calculator_;
        CalibrationBufferCounter buffer_counter_;

        unsigned int biggest_gap_current_;
        unsigned int acceleration_factor_;
        unsigned int last_line_;
        bool weights_invalid_;
    };

}

#endif
