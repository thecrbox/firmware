import esphome.codegen as cg
from esphome.components import i2c, ssd1306_base, text_sensor
from esphome.components.ssd1306_base import _validate
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_PAGES

CONF_DUMP_TARGET_TEXT_SENSOR = "dump_target_text_sensor"

AUTO_LOAD = ["ssd1306_base"]
DEPENDENCIES = ["i2c"]

ssd1306_dump = cg.esphome_ns.namespace("ssd1306_dump")
DUMPINGI2CSSD1306 = ssd1306_dump.class_("DUMPINGI2CSSD1306", ssd1306_base.SSD1306, i2c.I2CDevice)

CONFIG_SCHEMA = cv.All(
    ssd1306_base.SSD1306_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(DUMPINGI2CSSD1306),
            cv.Optional(CONF_DUMP_TARGET_TEXT_SENSOR): cv.use_id(text_sensor.TextSensor),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x3C)),
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
    _validate,
    )


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await ssd1306_base.setup_ssd1306(var, config)
    await i2c.register_i2c_device(var, config)
    if CONF_DUMP_TARGET_TEXT_SENSOR in config:
        text_sensor_ = await cg.get_variable(config[CONF_DUMP_TARGET_TEXT_SENSOR])
        cg.add(var.set_dump_target_text_sensor(text_sensor_))