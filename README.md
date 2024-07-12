# NeuronWirelessLibs
The set of libraries and sources for nRF MCU family version of the Neuron Platform. Please refer to the [NeuronWireless_defy][firmware:defy] and [NeuronWireless_raise2][firmware:raise2] repositories
for installation and usage instructions.

## Preparations
* Add this repository to your project as submodule `git submodule add -b "main" "git@github.com:Dygmalab/NeuronWirelessLibs.git" "libraries"`
* Update submodules `git submodule update --init --recursive`
* Download the Nordic Semiconductor SDK [nrf5_sdk_17.1.0_ddde560][sdk]
* Unpack the `nrf5_sdk_17.1.0_ddde560` into the `libraries/SDK/nRF5_SDK_17.1.0_ddde560` folder

 [firmware:defy]: https://github.com/Dygmalab/NeuronWireless_defy
 [firmware:raise2]: https://github.com/Dygmalab/NeuronWireless_raise2
 [sdk]: https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5_sdk_17.1.0_ddde560.zip
