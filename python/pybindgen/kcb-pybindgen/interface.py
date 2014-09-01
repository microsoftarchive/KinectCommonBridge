# encoding=utf-8

import os
import sys
from textwrap import wrap
import pybindgen

# from pybindgen import *

from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink, retval, param
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen.typehandlers.base import ReverseWrapperBase


class UCharBufferReturn(ReturnValue):
    CTYPES = []

    def __init__(self, ctype):
        super(UCharBufferReturn, self).__init__(ctype, is_const=False)

    def convert_c_to_python(self, wrapper):
        pybuf = wrapper.after_call.declare_variable("PyObject*", "pybuf")

        wrapper.after_call.write_code(
            "pybuf = PyCapsule_New(self->obj->" + wrapper.attribute_name + ", \"" + wrapper.attribute_name + "\", NULL);")

        # wrapper.after_call.write_code("if (!PyArg_ParseTuple(args,\"O\", pybuf)) {return NULL}")
        # 'wrapper.attribute_name' holds the name provided as a first argument to UCharBuffer return instance.
        # It is important that it is equal to the variable name so it can be resolved
        #wrapper.after_call.write_code("%s = PyBuffer_FromReadWriteMemory(%s, (%s)*sizeof(unsigned char));" % (pybuf, "self->obj->" + wrapper.attribute_name, self.length_expression))

        wrapper.build_params.add_parameter("N", [pybuf], prepend=True)

    def convert_python_to_c(self, wrapper):
         wrapper.parse_params.add_parameter("pybuf", ['&' + self.value])


def generate_module(out_file=sys.stdout):
    """
    Generates module based on method definitions descriptions
    :param out_file: file to hold generated output
    """
    mod = pybindgen.Module('KCBv2Lib')
    mod.add_include('"KCBv2Lib.h"')

    kcb_frame_description = mod.add_struct('KCBFrameDescription')
    kcb_frame_description.add_instance_attribute('width', 'int')
    kcb_frame_description.add_instance_attribute('height', 'int')
    kcb_frame_description.add_instance_attribute('horizontalFieldOfView', 'float')
    kcb_frame_description.add_instance_attribute('verticalFieldOfView', 'float')
    kcb_frame_description.add_instance_attribute('diagonalFieldOfView', 'float')
    kcb_frame_description.add_instance_attribute('lengthInPixels', 'unsigned int')
    kcb_frame_description.add_instance_attribute('bytesPerPixel', 'unsigned int')

    kcb_audio_frame = mod.add_struct('KCBAudioFrame')
    kcb_audio_frame.add_instance_attribute('cAudioBufferSize', 'unsigned long')  # ULONG
    kcb_audio_frame.add_instance_attribute('pAudioBuffer', UCharBufferReturn("unsigned char*"))  # byte*
    kcb_audio_frame.add_instance_attribute('ulBytesRead', 'unsigned long')  # ULONG
    kcb_audio_frame.add_instance_attribute('fBeamAngle', 'float')  # FLOAT
    kcb_audio_frame.add_instance_attribute('fBeamAngleConfidence', 'float')  # FLOAT

    # IBody : public IUnknown
    # {
    #     public:
    #         virtual HRESULT STDMETHODCALLTYPE GetJoints(
    #             /* [annotation] */
    #             _Pre_equal_to_(JointType_Count)  UINT capacity,
    #             /* [annotation][size_is][out] */
    #             _Out_writes_all_(capacity)  Joint *joints) = 0;
    #
    #         virtual HRESULT STDMETHODCALLTYPE GetJointOrientations(
    #             /* [annotation] */
    #             _Pre_equal_to_(JointType_Count)  UINT capacity,
    #             /* [annotation][size_is][out] */
    #             _Out_writes_all_(capacity)  JointOrientation *jointOrientations) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Engaged(
    #             /* [annotation][out][retval] */
    #             _Out_  DetectionResult *detectionResult) = 0;
    #
    #         virtual HRESULT STDMETHODCALLTYPE GetExpressionDetectionResults(
    #             /* [annotation] */
    #             _Pre_equal_to_(Expression_Count)  UINT capacity,
    #             /* [annotation][size_is][out] */
    #             _Out_writes_all_(capacity)  DetectionResult *detectionResults) = 0;
    #
    #         virtual HRESULT STDMETHODCALLTYPE GetActivityDetectionResults(
    #             /* [annotation] */
    #             _Pre_equal_to_(Activity_Count)  UINT capacity,
    #             /* [annotation][size_is][out] */
    #             _Out_writes_all_(capacity)  DetectionResult *detectionResults) = 0;
    #
    #         virtual HRESULT STDMETHODCALLTYPE GetAppearanceDetectionResults(
    #             /* [annotation] */
    #             _Pre_equal_to_(Appearance_Count)  UINT capacity,
    #             /* [annotation][size_is][out] */
    #             _Out_writes_all_(capacity)  DetectionResult *detectionResults) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_HandLeftState(
    #             /* [annotation][out][retval] */
    #             _Out_  HandState *handState) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_HandLeftConfidence(
    #             /* [annotation][out][retval] */
    #             _Out_  TrackingConfidence *confidence) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_HandRightState(
    #             /* [annotation][out][retval] */
    #             _Out_  HandState *handState) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_HandRightConfidence(
    #             /* [annotation][out][retval] */
    #             _Out_  TrackingConfidence *confidence) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ClippedEdges(
    #             /* [annotation][out][retval] */
    #             _Out_  DWORD *clippedEdges) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_TrackingId(
    #             /* [annotation][out][retval] */
    #             _Out_  UINT64 *trackingId) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_IsTracked(
    #             /* [annotation][out][retval] */
    #             _Out_  BOOLEAN *tracked) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_IsRestricted(
    #             /* [annotation][out][retval] */
    #             _Out_  BOOLEAN *isRestricted) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Lean(
    #             /* [annotation][out][retval] */
    #             _Out_  PointF *amount) = 0;
    #
    #         virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_LeanTrackingState(
    #             /* [annotation][out][retval] */
    #             _Out_  TrackingState *trackingState) = 0;
    #
    #     };

    # typedef struct KCBBodyFrame
    # {
    # UINT Count;
    #     IBody** Bodies;
    #     LONGLONG TimeStamp;
    # } KCBBodyFrame;

    kcb_body_index_frame = mod.add_struct('KCBBodyIndexFrame')
    kcb_body_index_frame.add_instance_attribute('Size', 'unsigned long')  # ULONG
    kcb_body_index_frame.add_instance_attribute('Buffer', UCharBufferReturn("unsigned char*"))  # byte*
    kcb_body_index_frame.add_instance_attribute('TimeStamp', 'long long')  # LONGLONG

    #kcb_color_frame = mod.add_struct('KCBColorFrame')
    ##kcb_color_frame.add_instance_attribute('ColorImageFormat', )  # ColorImageFormat
    #kcb_color_frame.add_instance_attribute('Size', 'unsigned long')  # ULONG
    #kcb_color_frame.add_instance_attribute('Buffer', UCharBufferReturn("unsigned char*", "colorBufferLen"))  # BYTE*
    #kcb_color_frame.add_instance_attribute('TimeStamp', 'long long')  # LONGLONG

    # kcb_depth_frame = mod.add_struct('KCBDepthFrame')
    # kcb_depth_frame.add_instance_attribute('Size', 'unsigned long')  # ULONG
    # # UINT16* == unsigned short* == unsigned short int*
    # kcb_depth_frame.add_instance_attribute('Buffer', UCharBufferReturn("unsigned short int*", "audioBufferLen"))
    # kcb_depth_frame.add_instance_attribute('TimeStamp', 'long long')  # LONGLONG

    mod.add_function('KCBOpenDefaultSensor', ReturnValue.new("int"), [])  # retval('KCBHANDLE')
    mod.add_function('KCBCloseSensor', ReturnValue.new('long'),
                     [Parameter.new('int*', 'kcbHandle', direction=Parameter.DIRECTION_IN, transfer_ownership=False)])

    mod.generate(FileCodeSink(out_file))


if __name__ == '__main__':
    generate_module(open("binding-gen.cpp", "wt"))
