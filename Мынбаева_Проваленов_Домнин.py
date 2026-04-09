import pandas as pd

ANSWER = './answer/'
file_names = ['MS-b1', 'MS-m1', 'MS-s1']

#Число проданных в этот день яблок и карандашей
def get_sold_items(sell):
    sell['sku_num'] = sell['sku_num'].apply(lambda x: -1 if x.find('ap') == 6 else 1)
    sell['num'] = sell['sku_num']
    sell['sku_num'] = sell['sku_num'].apply(lambda x: 0 if x > 0 else -x)
    sell['num'] = sell['num'].apply(lambda x: 0 if x < 0 else x)
    sell = sell.rename(columns={"sku_num": "apple", "num": "pen"})
    sell = sell.groupby(['date']).sum()
    return sell

# Состояние склада на каждый день
def daily_inventory(supply, sell):
    daily_inv = pd.DataFrame(columns=['apple', 'pen'], index=sell.index)
    daily_inv['apple'] = -sell['apple']
    daily_inv['pen'] = -sell['pen']
    daily_inv = pd.concat([daily_inv, supply]) #присоединяем таблицу с поставками
    # Сгруппируем записи по дате и сложим строки в каждой группе
    daily_inv = daily_inv.groupby(['date']).sum()
    daily_inv['apple'] = daily_inv['apple'].cumsum()
    daily_inv['pen'] = daily_inv['pen'].cumsum()
    return daily_inv

# Cуммарное количество товара, украденного к концу данного месяца.
def monthly_theft(daily_inv, inventory):
    month_inv = daily_inv.resample('M').last()
    month_inv['pen'] -= inventory['pen']
    month_inv['apple'] -= inventory['apple']
    month_inv['pen'] = month_inv['pen'] - month_inv['pen'].shift(1).fillna(0)
    month_inv['apple'] = month_inv['apple'] - month_inv['apple'].shift(1).fillna(0)
    return month_inv


def update(state, sell, steal, dframe):
    sold = sell.resample('Y').sum()
    # Изменяем имена колонок apple и pen.
    sold = sold.rename(columns={"apple": "apple_sold", "pen": "pen_sold"})
    sold['state'] = state
    sold['year'] = sold.index
    sold['year'] = sold['year'].apply(lambda x: x.to_pydatetime().year)
    sold.index = range(0, len(sold.index))

    # Аналогично с украденными товарами.
    stolen = steal.resample('Y').sum()
    stolen['state'] = state
    stolen = stolen.rename(columns={"apple": "apple_stolen", "pen": "pen_stolen"})
    stolen['year'] = stolen.index
    stolen['year'] = stolen['year'].apply(lambda x: x.to_pydatetime().year)
    stolen.index = range(0, len(stolen.index))

    # Объединим таблицы
    year_data = pd.concat([stolen, sold], ignore_index=True, sort=False).groupby(['year']).sum()
    year_data['year'] = year_data.index
    year_data['state'] = state
    year_data.index = range(0, len(year_data.index))

    updated = pd.concat([dframe, year_data], ignore_index=True, sort=False).groupby(['year', 'state'], as_index=False)[[
        'apple_sold',
        'apple_stolen',
        'pen_sold',
        'pen_stolen']].sum()
    return updated


def main(name, dframe):
    j = 'input/' + name
    supply = pd.read_csv(j + '-supply.csv', sep=',')
    supply.index = supply.date
    supply = supply.drop('date', axis=1)
    sell = pd.read_csv(j + '-sell.csv', sep=',')
    inventory = pd.read_csv(j + '-inventory.csv', sep=',')
    inventory.index = inventory.date
    inventory = inventory.drop('date', axis=1)
    sell = get_sold_items(sell)
    sell.index = pd.to_datetime(sell.index)
    inventory.index = pd.to_datetime(inventory.index)
    supply.index = pd.to_datetime(supply.index)

    di = daily_inventory(supply, sell)
    di.to_csv(ANSWER + name + '-daily.csv', index=True)
    print(name + '[daily_inventory]:\n', di.head(), "\n")
    mt = monthly_theft(di, inventory)
    mt.to_csv(ANSWER + name + '-steal.csv', index=True)
    print(name + '[monthly_theft]:\n', mt.head(), "\n")
    return update(name[0:2], sell, mt, dframe)


aggregated_data = pd.DataFrame(columns=['year', 'state',
                                        'apple_sold', 'apple_stolen',
                                        'pen_sold', 'pen_stolen'])
for e in file_names:
    aggregated_data = main(e, aggregated_data)
    print('\n----------------------------------------\n')
aggregated_data.to_csv(ANSWER + 'final.csv', index=True)
print(aggregated_data)
